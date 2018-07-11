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
#ifndef __ANTARES_LIBS_STUDY_PARTS_SOLAR_PREPRO_HXX__
# define __ANTARES_LIBS_STUDY_PARTS_SOLAR_PREPRO_HXX__

# include "../../study.h"


namespace Antares
{
namespace Data
{
namespace Solar
{

	inline void Prepro::estimateMemoryUsage(StudyMemoryUsage& u) const
	{
		xcast.estimateMemoryUsage(u);
	}


	inline Yuni::uint64 Prepro::memoryUsage() const
	{
		return xcast.memoryUsage();
	}



	inline bool Prepro::invalidate(bool reload) const
	{
		return xcast.invalidate(reload);
	}


	inline void Prepro::markAsModified() const
	{
		xcast.markAsModified();
	}


	inline void Prepro::resetToDefault()
	{
		xcast.resetToDefaultValues();
	}




} // namespace Solar
} // namespace Data
} // namespace Antares

#endif // __ANTARES_LIBS_STUDY_PARTS_SOLAR_PREPRO_H__
