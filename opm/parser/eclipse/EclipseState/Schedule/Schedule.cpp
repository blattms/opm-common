/*
  Copyright 2013 Statoil ASA.

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

#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>


namespace Opm {

    Schedule::Schedule(DeckConstPtr deck) {
        if (deck->hasKeyword("SCHEDULE")) {
            addGroup( "FIELD" );
            initFromDeck(deck);
        } else
            throw std::invalid_argument("Deck does not contain SCHEDULE section.\n");
    }

    void Schedule::createTimeMap(DeckConstPtr deck) {
        boost::gregorian::date startDate(defaultStartDate);
        if (deck->hasKeyword("START")) {
            DeckKeywordConstPtr startKeyword = deck->getKeyword("START");
            startDate = TimeMap::dateFromEclipse(startKeyword->getRecord(0));
        }

        m_timeMap.reset(new TimeMap(startDate));
    }

    void Schedule::initFromDeck(DeckConstPtr deck) {
        createTimeMap(deck);
        iterateScheduleSection(deck);
    }


    void Schedule::iterateScheduleSection(DeckConstPtr deck) {
        DeckKeywordConstPtr scheduleKeyword = deck->getKeyword("SCHEDULE");
        size_t deckIndex = scheduleKeyword->getDeckIndex() + 1;
        size_t currentStep = 0;
        while (deckIndex < deck->size()) {

            DeckKeywordConstPtr keyword = deck->getKeyword(deckIndex);

            if (keyword->name() == "DATES") {
                handleDATES(keyword);
                currentStep += keyword->size();
            }

            if (keyword->name() == "TSTEP") {
                handleTSTEP(keyword);
                currentStep += keyword->getRecord(0)->getItem(0)->size(); // This is a bit weird API.
            }

            if (keyword->name() == "WELSPECS")
                handleWELSPECS(keyword);

            if (keyword->name() == "WCONHIST")
                handleWCONHIST(keyword, currentStep);

            if (keyword->name() == "WCONPROD")
                handleWCONPROD(keyword, currentStep);

            if (keyword->name() == "WCONINJE")
                handleWCONINJE(keyword, currentStep);

            if (keyword->name() == "WCONINJH")
                handleWCONINJH(keyword, currentStep);

            if (keyword->name() == "COMPDAT") 
                handleCOMPDAT( keyword , currentStep );

            if (keyword->name() == "GCONINJE")
                handleGCONINJE( keyword , currentStep );

            if (keyword->name() == "GCONPROD")
                handleGCONPROD( keyword , currentStep );

            deckIndex++;
        }
    }

    void Schedule::handleDATES(DeckKeywordConstPtr keyword) {
        m_timeMap->addFromDATESKeyword(keyword);
    }

    void Schedule::handleTSTEP(DeckKeywordConstPtr keyword) {
        m_timeMap->addFromTSTEPKeyword(keyword);
    }

    void Schedule::handleWELSPECS(DeckKeywordConstPtr keyword) {
        for (size_t recordNr = 0; recordNr < keyword->size(); recordNr++) {
            DeckRecordConstPtr record = keyword->getRecord(recordNr);
            const std::string& wellName = record->getItem("WELL")->getString(0);
            const std::string& groupName = record->getItem("GROUP")->getString(0);

            if (!hasGroup(groupName)) {
                addGroup(groupName);
            }

            if (!hasWell(wellName)) {
                addWell(wellName);
            }
            WellPtr well = getWell(wellName);
            well->addWELSPECS(record);
        }
    }

    void Schedule::handleWCONProducer(DeckKeywordConstPtr keyword, size_t currentStep, bool isPredictionMode) {
        for (size_t recordNr = 0; recordNr < keyword->size(); recordNr++) {
            DeckRecordConstPtr record = keyword->getRecord(recordNr);
            const std::string& wellName = record->getItem("WELL")->getString(0);
            WellPtr well = getWell(wellName);
            double orat  = record->getItem("ORAT")->getDouble(0);
            double wrat  = record->getItem("WRAT")->getDouble(0);
            double grat  = record->getItem("GRAT")->getDouble(0);
            
            well->setOilRate(currentStep, orat);
            well->setWaterRate(currentStep, wrat);
            well->setGasRate(currentStep, grat);
            well->setInPredictionMode(currentStep, isPredictionMode);
        }
    }

    void Schedule::handleWCONHIST(DeckKeywordConstPtr keyword, size_t currentStep) {
        handleWCONProducer(keyword, currentStep, false);
    }

    void Schedule::handleWCONPROD(DeckKeywordConstPtr keyword, size_t currentStep) {
        handleWCONProducer(keyword, currentStep, true);
    }

    void Schedule::handleWCONINJE(DeckKeywordConstPtr keyword, size_t currentStep) {
        for (size_t recordNr = 0; recordNr < keyword->size(); recordNr++) {
            DeckRecordConstPtr record = keyword->getRecord(recordNr);
            const std::string& wellName = record->getItem("WELL")->getString(0);
            WellPtr well = getWell(wellName);
            double injectionRate  = record->getItem("SURFACE_FLOW_TARGET")->getDouble(0);
            
            well->setInjectionRate( currentStep , injectionRate );
            well->setInPredictionMode(currentStep, true);
        }
    }

    void Schedule::handleWCONINJH(DeckKeywordConstPtr keyword, size_t currentStep) {
        for (size_t recordNr = 0; recordNr < keyword->size(); recordNr++) {
            DeckRecordConstPtr record = keyword->getRecord(recordNr);
            const std::string& wellName = record->getItem("WELL")->getString(0);
            WellPtr well = getWell(wellName);
            double injectionRate  = record->getItem("RATE")->getDouble(0);
            
            well->setInjectionRate( currentStep , injectionRate );
            well->setInPredictionMode(currentStep, false );
        }
    }


    void Schedule::handleGCONINJE(DeckKeywordConstPtr keyword, size_t currentStep) {
        for (size_t recordNr = 0; recordNr < keyword->size(); recordNr++) {
            DeckRecordConstPtr record = keyword->getRecord(recordNr);
            const std::string& groupName = record->getItem("GROUP")->getString(0);
            GroupPtr group = getGroup(groupName);

            {
                PhaseEnum phase = PhaseEnumFromString( record->getItem("PHASE")->getString(0) );
                group->setInjectionPhase( currentStep , phase );
            }
            {
                GroupInjection::ControlEnum controlMode = GroupInjection::ControlEnumFromString( record->getItem("CONTROL_MODE")->getString(0) );
                group->setInjectionControlMode( currentStep , controlMode );
            }
            group->setSurfaceMaxRate( currentStep , record->getItem("SURFACE_TARGET")->getDouble(0));
            group->setReservoirMaxRate( currentStep , record->getItem("RESV_TARGET")->getDouble(0));
            group->setTargetReinjectFraction( currentStep , record->getItem("REINJ_TARGET")->getDouble(0));
            group->setTargetVoidReplacementFraction( currentStep , record->getItem("VOIDAGE_TARGET")->getDouble(0));
        }
    }


    void Schedule::handleGCONPROD(DeckKeywordConstPtr keyword, size_t currentStep) {
        for (size_t recordNr = 0; recordNr < keyword->size(); recordNr++) {
            DeckRecordConstPtr record = keyword->getRecord(recordNr);
            const std::string& groupName = record->getItem("GROUP")->getString(0);
            GroupPtr group = getGroup(groupName);
            {
                GroupProduction::ControlEnum controlMode = GroupProduction::ControlEnumFromString( record->getItem("CONTROL_MODE")->getString(0) );
                group->setProductionControlMode( currentStep , controlMode );
            }
            group->setOilTargetRate( currentStep , record->getItem("OIL_TARGET")->getDouble(0));
            group->setGasTargetRate( currentStep , record->getItem("GAS_TARGET")->getDouble(0));
            group->setWaterTargetRate( currentStep , record->getItem("WATER_TARGET")->getDouble(0));
            group->setLiquidTargetRate( currentStep , record->getItem("LIQUID_TARGET")->getDouble(0));
            {
                GroupProductionExceedLimit::ActionEnum exceedAction = GroupProductionExceedLimit::ActionEnumFromString(record->getItem("EXCEED_PROC")->getString(0) );
                group->setProductionExceedLimitAction( currentStep , exceedAction );
            }
            
        }
    }



    void Schedule::handleCOMPDAT(DeckKeywordConstPtr keyword , size_t currentStep) {
        std::map<std::string , std::vector< CompletionConstPtr> > completionMapList = Completion::completionsFromCOMPDATKeyword( keyword );
        std::map<std::string , std::vector< CompletionConstPtr> >::iterator iter;
        
        for( iter= completionMapList.begin(); iter != completionMapList.end(); iter++) {
            const std::string wellName = iter->first;
            WellPtr well = getWell( wellName );

            well->addCompletions( currentStep , iter->second );
        }
    }

    boost::gregorian::date Schedule::getStartDate() const {
        return m_timeMap->getStartDate();
    }

    TimeMapConstPtr Schedule::getTimeMap() const {
        return m_timeMap;
    }

    void Schedule::addWell(const std::string& wellName) {
        WellPtr well(new Well(wellName, m_timeMap));
        m_wells[ wellName ] = well;
    }

    size_t Schedule::numWells() const {
        return m_wells.size();
    }

    bool Schedule::hasWell(const std::string& wellName) const {
        return m_wells.find(wellName) != m_wells.end();
    }

    WellPtr Schedule::getWell(const std::string& wellName) const {
        if (hasWell(wellName)) {
            return m_wells.at(wellName);
        } else
            throw std::invalid_argument("Well: " + wellName + " does not exist");
    }

    void Schedule::addGroup(const std::string& groupName) {
        GroupPtr group(new Group(groupName, m_timeMap));
        m_groups[ groupName ] = group;
    }

    size_t Schedule::numGroups() const {
        return m_groups.size();
    }

    bool Schedule::hasGroup(const std::string& groupName) const {
        return m_groups.find(groupName) != m_groups.end();
    }

    GroupPtr Schedule::getGroup(const std::string& groupName) const {
        if (hasGroup(groupName)) {
            return m_groups.at(groupName);
        } else
            throw std::invalid_argument("Group: " + groupName + " does not exist");
    }



}
