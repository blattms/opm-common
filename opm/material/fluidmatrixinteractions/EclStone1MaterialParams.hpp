// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*
  Copyright (C) 2015 by Andreas Lauser

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/
/*!
 * \file
 * \copydoc Opm::EclStone1MaterialParams
 */
#ifndef OPM_ECL_STONE1_MATERIAL_PARAMS_HPP
#define OPM_ECL_STONE1_MATERIAL_PARAMS_HPP

#include <type_traits>
#include <cassert>
#include <memory>

namespace Opm {

/*!
 * \brief Default implementation for the parameters required by the
 *        three-phase capillary pressure/relperm Stone 2 model used by
 *        Eclipse.
 *
 * Essentially, this class just stores the two parameter objects for
 * the twophase capillary pressure laws.
 */
template<class Traits, class GasOilParamsT, class OilWaterParamsT>
class EclStone1MaterialParams
{
    typedef typename Traits::Scalar Scalar;
    enum { numPhases = 3 };

public:
    typedef GasOilParamsT GasOilParams;
    typedef OilWaterParamsT OilWaterParams;

    /*!
     * \brief The default constructor.
     */
    EclStone1MaterialParams()
    {
#ifndef NDEBUG
        finalized_ = false;
#endif
    }

    /*!
     * \brief Finish the initialization of the parameter object.
     */
    void finalize()
    {
#ifndef NDEBUG
        finalized_ = true;
#endif
    }

    /*!
     * \brief The parameter object for the gas-oil twophase law.
     */
    const GasOilParams& gasOilParams() const
    { assertFinalized_(); return *gasOilParams_; }

    /*!
     * \brief The parameter object for the gas-oil twophase law.
     */
    GasOilParams& gasOilParams()
    { assertFinalized_(); return *gasOilParams_; }

    /*!
     * \brief Set the parameter object for the gas-oil twophase law.
     */
    void setGasOilParams(std::shared_ptr<GasOilParams> val)
    { gasOilParams_ = val; }

    /*!
     * \brief The parameter object for the oil-water twophase law.
     */
    const OilWaterParams& oilWaterParams() const
    { assertFinalized_(); return *oilWaterParams_; }

    /*!
     * \brief The parameter object for the oil-water twophase law.
     */
    OilWaterParams& oilWaterParams()
    { assertFinalized_(); return *oilWaterParams_; }

    /*!
     * \brief Set the parameter object for the oil-water twophase law.
     */
    void setOilWaterParams(std::shared_ptr<OilWaterParams> val)
    { oilWaterParams_ = val; }

    /*!
     * \brief Set the saturation of "connate" water.
     *
     * According to
     *
     * http://www.glossary.oilfield.slb.com/en/Terms/c/connate_water.aspx
     *
     * the connate water is the water which is trapped in the pores of the rock during
     * the rock's formation. For our application, this is basically a reduction of the
     * rock's porosity...
     */
    void setSwl(Scalar val)
    { Swl_ = val; }

    /*!
     * \brief Return the saturation of "connate" water.
     */
    Scalar Swl() const
    { assertFinalized_(); return Swl_; }

    /*!
     * \brief Set the critical saturation of oil in the water-oil system.
     */
    void setSowcr(Scalar val)
    { Sowcr_ = val; }

    /*!
     * \brief Return the critical saturation of oil in the water-oil system.
     */
    Scalar Sowcr() const
    { assertFinalized_(); return Sowcr_; }

    /*!
     * \brief Set the critical saturation of oil in the oil-gas system.
     */
    void setSogcr(Scalar val)
    { Sogcr_ = val; }

    /*!
     * \brief Return the critical saturation of oil in the oil-gas system.
     */
    Scalar Sogcr() const
    { assertFinalized_(); return Sogcr_; }

    /*!
     * \brief Set the exponent of the extended Stone 1 model.
     */
    void setEta(Scalar val)
    { eta_ = val; }

    /*!
     * \brief Return the exponent of the extended Stone 1 model.
     */
    Scalar eta() const
    { assertFinalized_(); return eta_; }

private:
#ifndef NDEBUG
    void assertFinalized_() const
    { assert(finalized_); }

    bool finalized_;
#else
    void assertFinalized_() const
    { }
#endif

    std::shared_ptr<GasOilParams> gasOilParams_;
    std::shared_ptr<OilWaterParams> oilWaterParams_;

    Scalar Swl_;
    Scalar Sowcr_;
    Scalar Sogcr_;
    Scalar eta_;
};
} // namespace Opm

#endif
