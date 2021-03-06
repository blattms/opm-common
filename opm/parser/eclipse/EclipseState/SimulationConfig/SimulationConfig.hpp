/*
  Copyright 2015 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OPM_SIMULATION_CONFIG_HPP
#define OPM_SIMULATION_CONFIG_HPP

#include <opm/parser/eclipse/EclipseState/SimulationConfig/RockConfig.hpp>
#include <opm/parser/eclipse/EclipseState/SimulationConfig/ThresholdPressure.hpp>
#include <opm/parser/eclipse/EclipseState/SimulationConfig/BCConfig.hpp>

namespace Opm {

    class Deck;
    class FieldPropsManager;

    class SimulationConfig {

    public:

        SimulationConfig();
        SimulationConfig(bool restart,
                         const Deck& deck,
                         const FieldPropsManager& fp);

        static SimulationConfig serializeObject();

        const RockConfig& rock_config() const;
        const ThresholdPressure& getThresholdPressure() const;
        const BCConfig& bcconfig() const;
        bool useThresholdPressure() const;
        bool useCPR() const;
        bool hasDISGAS() const;
        bool hasVAPOIL() const;
        bool isThermal() const;
        bool isDiffusive() const;

        bool operator==(const SimulationConfig& data) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            m_ThresholdPressure.serializeOp(serializer);
            m_bcconfig.serializeOp(serializer);
            m_rock_config.serializeOp(serializer);
            serializer(m_useCPR);
            serializer(m_DISGAS);
            serializer(m_VAPOIL);
            serializer(m_isThermal);
            serializer(m_diffuse);
        }

    private:
        ThresholdPressure m_ThresholdPressure;
        BCConfig m_bcconfig;
        RockConfig m_rock_config;
        bool m_useCPR;
        bool m_DISGAS;
        bool m_VAPOIL;
        bool m_isThermal;
        bool m_diffuse;
    };

} //namespace Opm



#endif
