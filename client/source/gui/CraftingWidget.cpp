/*
 * =====================================================================================
 *
 *  OpenMiner
 *  Copyright (C) 2018-2020 Unarelith, Quentin Bazin <openminer@unarelith.net>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * =====================================================================================
 */
#include "CraftingWidget.hpp"
#include "Registry.hpp"

CraftingWidget::CraftingWidget(ClientCommandHandler &client, Inventory &craftingInventory, Widget *parent)
	: Widget(parent), m_client(client), m_craftingInventory(craftingInventory)
{
}

void CraftingWidget::init(unsigned int offset, unsigned int size) {
	m_craftingInventoryWidget.init(m_craftingInventory, offset, size * size);
	m_craftingInventoryWidget.setPosition(29, 16, 0);

	m_craftingResultInventoryWidget.init(m_craftingResultInventory);
	m_craftingResultInventoryWidget.setPosition(123, 34, 0);
}

void CraftingWidget::onMouseEvent(const SDL_Event &event, MouseItemWidget &mouseItemWidget) {
	m_craftingInventoryWidget.onMouseEvent(event, mouseItemWidget);
	m_craftingResultInventoryWidget.onMouseEvent(event, mouseItemWidget, true);

	if (m_recipe && !m_craftingResultInventory.getStack(0, 0).item().id()) {
		for (u8 x = 0 ; x < m_craftingInventory.width() ; ++x) {
			for (u8 y = 0 ; y < m_craftingInventory.height() ; ++y) {
				const ItemStack &stack = m_craftingInventory.getStack(x, y);
				if (stack.item().id()) {
					m_craftingInventory.setStack(x, y, (stack.amount() > 1) ? stack.item().stringID() : "", stack.amount() - 1);
				}
			}
		}

		m_craftingInventoryWidget.init(m_craftingInventory);

		m_recipe = nullptr;
	}
}

void CraftingWidget::update() {
	const Recipe *recipe = Registry::getInstance().getRecipe(m_craftingInventory);
	if (recipe && recipe->type() != "craft")
		recipe = nullptr;

	if ((!m_recipe || m_recipe != recipe)) {
		m_recipe = recipe;

		if (m_recipe)
			m_craftingResultInventory.setStack(0, 0, m_recipe->result().item().stringID(), m_recipe->result().amount());
		else
			m_craftingResultInventory.setStack(0, 0, "", 0);

		m_craftingResultInventoryWidget.init(m_craftingResultInventory);
	}
}

void CraftingWidget::draw(gk::RenderTarget &target, gk::RenderStates states) const {
	states.transform *= getTransform();

	target.draw(m_craftingInventoryWidget, states);
	target.draw(m_craftingResultInventoryWidget, states);
}

