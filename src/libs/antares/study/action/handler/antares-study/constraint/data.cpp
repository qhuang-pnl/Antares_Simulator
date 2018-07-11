/*
** Copyright 2007-2018 RTE
** Authors: Antares_Simulator Team
**
** This file is part of Antares_Simulator.
**
** Antares_Simulator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** There are special exceptions to the terms and conditions of the
** license as they are applied to this software. View the full text of
** the exceptions in file COPYING.txt in the directory of this software
** distribution
**
** Antares_Simulator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Antares_Simulator. If not, see <http://www.gnu.org/licenses/>.
**
** SPDX-License-Identifier: licenceRef-GPL3_WITH_RTE-Exceptions
*/

#include "data.h"


namespace Antares
{
namespace Action
{
namespace AntaresStudy
{
namespace Constraint
{


	bool Data::prepareWL(Context&)
	{
		pInfos.message.clear();
		pInfos.state = stReady;
		switch (pInfos.behavior)
		{
			case bhOverwrite:
				pInfos.message << "The Bounds/RHS will be copied";
				break;
			default:
				pInfos.state = stNothingToDo;
				break;
		}

		return true;
	}


	bool Data::performWL(Context& ctx)
	{
		if (ctx.constraint && ctx.extStudy)
		{
			Antares::Data::ConstraintName id;
			TransformNameIntoID(pOriginalConstraintName, id);

			Antares::Data::BindingConstraint* source = ctx.extStudy->bindingConstraints.find(id);

			if (source && source != ctx.constraint)
			{
				source->matrix().invalidate(true);
				assert(source->matrix().width > 0);
				assert(source->matrix().height > 0);
				ctx.constraint->matrix() = source->matrix();
				source->matrix().unloadFromMemory();
				return true;
			}
		}
		return false;
	}





} // namespace Constraint
} // namespace AntaresStudy
} // namespace Action
} // namespace Antares

