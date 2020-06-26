/*
 * =====================================================================================
 *
 *  OpenMiner
 *
 *  Copyright (C) 2018-2020 Unarelith, Quentin Bazin <openminer@unarelith.net>
 *  Copyright (C) 2019-2020 the OpenMiner contributors (see CONTRIBUTORS.md)
 *
 *  This file is part of OpenMiner.
 *
 *  OpenMiner is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  OpenMiner is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with OpenMiner; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * =====================================================================================
 */
#include <gk/core/ApplicationStateStack.hpp>
#include <gk/core/Debug.hpp>

#include <filesystem.hpp>

#include "Config.hpp"
#include "TitleScreenState.hpp"
#include "WorldCreationState.hpp"
#include "WorldMenuState.hpp"
#include "WorldSelectionState.hpp"

namespace fs = ghc::filesystem;

WorldSelectionState::WorldSelectionState(TitleScreenState *titleScreen)
	: InterfaceState(titleScreen), m_titleScreen(titleScreen)
{
	m_title.setScale(Config::guiScale, Config::guiScale);
	m_title.setString("Select or create a world");
	m_title.updateVertexBuffer();

	m_background.setScale(Config::guiScale * 2, Config::guiScale * 2);

	m_filter1.setFillColor(gk::Color(0, 0, 0, 128));
	m_filter2.setFillColor(gk::Color(0, 0, 0, 192));

	m_menuWidget.setScale(Config::guiScale, Config::guiScale);
	m_menuWidget.addButton("New world", [this](TextButton &) {
		m_stateStack->push<WorldCreationState>(m_titleScreen);
	});

	m_cancelButton.setScale(Config::guiScale, Config::guiScale);
	m_cancelButton.setText("Cancel");
	m_cancelButton.setCallback([this](TextButton &) {
		m_stateStack->pop();
	});

	updateWidgetPosition();
	loadSaveList();
}

void WorldSelectionState::onEvent(const sf::Event &event) {
	InterfaceState::onEvent(event);

	if (event.type == sf::Event::Resized && !m_stateStack->empty() && &m_stateStack->top() != this) {
		m_menuWidget.onEvent(event);
	}

	if (!m_stateStack->empty() && &m_stateStack->top() == this) {
		m_menuWidget.onEvent(event);
		m_cancelButton.onEvent(event);
	}
}

void WorldSelectionState::update() {
}

void WorldSelectionState::updateWidgetPosition() {
	m_background.setPosRect(0, 0, Config::screenWidth / m_background.getScale().x, Config::screenHeight / m_background.getScale().y);
	m_background.setClipRect(0, 0, Config::screenWidth / m_background.getScale().x, Config::screenHeight / m_background.getScale().y);

	m_filter1.setSize(Config::screenWidth, Config::screenHeight);

	const int borderSize = 25 * Config::guiScale;
	m_filter2.setSize(Config::screenWidth, Config::screenHeight - borderSize * 2);
	m_filter2.setPosition(0, borderSize);

	m_cancelButton.setPosition(
		Config::screenWidth / 2.0f - m_cancelButton.getGlobalBounds().sizeX / 2.0f,
		Config::screenHeight - borderSize / 2 - m_cancelButton.getGlobalBounds().sizeY / 2.0f
	);

	m_title.setPosition(
		Config::screenWidth / 2.0f - m_title.getSize().x * Config::guiScale / 2.0f,
		borderSize / 2 - m_title.getSize().y * Config::guiScale / 2.0f
	);
}

void WorldSelectionState::loadSaveList() {
	if (fs::is_directory("saves")) {
		std::vector<fs::directory_entry> files;

		fs::path basePath = fs::current_path();
		fs::directory_iterator dir("saves/");
		for (const auto &entry : dir) {
			if (entry.is_regular_file() && entry.path().filename().generic_string()[0] != '_') {
				files.emplace_back(entry);
			}
		}

		std::sort(files.begin(), files.end());

		for (auto &entry : files) {
			std::string filename = entry.path().filename();
			if (filename.substr(filename.find_last_of('.')) == ".dat") {
				std::string saveFile = filename.substr(0, filename.find_last_of('.'));
				std::string filesize = std::to_string(entry.file_size() / 1000.f / 1000.f);
				m_menuWidget.addButton("- " + saveFile + " (" + filesize.substr(0, filesize.find_first_of('.') + 3) + " MB)" + " -", [&, saveFile](TextButton &) {
					m_stateStack->push<WorldMenuState>(saveFile, m_titleScreen);
				});
			}
		}
	}
}

void WorldSelectionState::draw(gk::RenderTarget &target, gk::RenderStates states) const {
	if (&m_stateStack->top() == this) {
		prepareDraw(target, states);

		target.draw(m_background, states);
		target.draw(m_filter1, states);
		target.draw(m_filter2, states);

		target.draw(m_title, states);

		target.draw(m_menuWidget, states);
		target.draw(m_cancelButton, states);
	}
}

