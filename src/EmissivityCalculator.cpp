/*******************************************************************************
 * This file is part of CMacIonize
 * Copyright (C) 2016 Bert Vandenbroucke (bert.vandenbroucke@gmail.com)
 *
 * CMacIonize is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CMacIonize is distributed in the hope that it will be useful,
 * but WITOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with CMacIonize. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

/**
 * @file EmissivityCalculator.cpp
 *
 * @brief EmissivityCalculator implementation.
 *
 * @author Bert Vandenbroucke (bv7@st-andrews.ac.uk)
 */
#include "EmissivityCalculator.hpp"
#include "Abundances.hpp"
#include "DensityValues.hpp"
#include "LineCoolingData.hpp"
#include "Utilities.hpp"
#include <cmath>

/**
 * @brief Constructor.
 *
 * Initializes hydrogen and helium continuous emission coefficient tables.
 */
EmissivityCalculator::EmissivityCalculator() {
  double ttab[8] = {4.e3, 6.e3, 8.e3, 1.e4, 1.2e4, 1.4e4, 1.6e4, 1.8e4};
  double hplt[8] = {0.162, 0.584, 1.046, 1.437, 1.742, 1.977, 2.159, 2.297};
  double hmit[8] = {92.6, 50.9, 33.8, 24.8, 19.53, 16.09, 13.7, 11.96};
  double heplt[8] = {0.189, 0.622, 1.076, 1.45, 1.74, 1.963, 2.14, 2.27};
  double hemit[8] = {15.7, 9.23, 6.71, 5.49, 4.83, 4.41, 4.135, 3.94};

  for (unsigned int i = 0; i < 8; ++i) {
    _logttab[i] = std::log(ttab[i]);
    _loghplt[i] = std::log(hplt[i]);
    _loghmit[i] = std::log(hmit[i]);
    _logheplt[i] = std::log(heplt[i]);
    _loghemit[i] = std::log(hemit[i]);
  }
}

/**
 * @brief Calculate the hydrogen and helium continuous emission coefficients at
 * the given temperature.
 *
 * @param T Temperature (in K).
 * @param emhpl Hydrogen coefficient 1.
 * @param emhmi Hydrogen coefficient 2.
 * @param emhepl Helium coefficient 1.
 * @param emhemi Helium coefficient 2.
 */
void EmissivityCalculator::bjump(double T, double emhpl, double emhmi,
                                 double emhepl, double emhemi) {
  double logt = std::log(T);

  int i = Utilities::locate(logt, _logttab, 8);
  i = std::max(i, 0);
  i = std::min(i, 7);

  emhpl = _loghplt[i] +
          (logt - _logttab[i]) * (_loghplt[i + 1] - _loghplt[i]) /
              (_logttab[i + 1] - _logttab[i]);
  emhmi = _loghmit[i] +
          (logt - _logttab[i]) * (_loghmit[i + 1] - _loghmit[i]) /
              (_logttab[i + 1] - _logttab[i]);
  emhepl = _logheplt[i] +
           (logt - _logttab[i]) * (_logheplt[i + 1] - _logheplt[i]) /
               (_logttab[i + 1] - _logttab[i]);
  emhemi = _loghemit[i] +
           (logt - _logttab[i]) * (_loghemit[i + 1] - _loghemit[i]) /
               (_logttab[i + 1] - _logttab[i]);

  emhpl = std::exp(emhpl) * 1.e-20 * 3.e18 / 3681. / 3681.;
  emhmi = std::exp(emhmi) * 1.e-20 * 3.e18 / 3643. / 3643.;
  emhepl = std::exp(emhepl) * 1.e-20 * 3.e18 / 3681. / 3681.;
  emhemi = std::exp(emhemi) * 1.e-20 * 3.e18 / 3643. / 3643.;
}

/**
 * @brief Calculate the emissivity values for a single cell.
 *
 * @param cell DensityValues of the cell.
 * @param abundances Abundances.
 * @param lines LineCoolingData used to calculate emission line strengths.
 * @return EmissivityValues in the cell.
 */
EmissivityValues EmissivityCalculator::calculate_emissivities(
    DensityValues &cell, Abundances &abundances, LineCoolingData &lines) {
  const double h0max = 0.25;

  EmissivityValues eval;

  if (cell.get_ionic_fraction(ION_H_n) < h0max &&
      cell.get_temperature() > 3000.) {
    double nhp =
        cell.get_total_density() * (1. - cell.get_ionic_fraction(ION_H_n));
    double nhep = cell.get_total_density() *
                  (1. - cell.get_ionic_fraction(ION_He_n)) *
                  abundances.get_abundance(ELEMENT_He);
    double ne = nhp + nhep;

    double abund[12];
    abund[0] =
        abundances.get_abundance(ELEMENT_N) *
        (1. - cell.get_ionic_fraction(ION_N_n) -
         cell.get_ionic_fraction(ION_N_p1) - cell.get_ionic_fraction(ION_N_p2));
    abund[1] =
        abundances.get_abundance(ELEMENT_N) * cell.get_ionic_fraction(ION_N_n);
    abund[2] = abundances.get_abundance(ELEMENT_O) *
               (1. - cell.get_ionic_fraction(ION_O_n) -
                cell.get_ionic_fraction(ION_O_p1));
    abund[3] =
        abundances.get_abundance(ELEMENT_O) * cell.get_ionic_fraction(ION_O_n);
    abund[4] =
        abundances.get_abundance(ELEMENT_O) * cell.get_ionic_fraction(ION_O_p1);
    abund[5] = abundances.get_abundance(ELEMENT_Ne) *
               cell.get_ionic_fraction(ION_Ne_p1);
    abund[6] =
        abundances.get_abundance(ELEMENT_S) *
        (1. - cell.get_ionic_fraction(ION_S_p1) -
         cell.get_ionic_fraction(ION_S_p2) - cell.get_ionic_fraction(ION_S_p3));
    abund[7] =
        abundances.get_abundance(ELEMENT_S) * cell.get_ionic_fraction(ION_S_p1);
    abund[8] = abundances.get_abundance(ELEMENT_C) *
               (1. - cell.get_ionic_fraction(ION_C_p1) -
                cell.get_ionic_fraction(ION_C_p2));
    abund[9] =
        abundances.get_abundance(ELEMENT_C) * cell.get_ionic_fraction(ION_C_p1);
    abund[10] =
        abundances.get_abundance(ELEMENT_N) * cell.get_ionic_fraction(ION_N_p1);
    abund[11] = abundances.get_abundance(ELEMENT_Ne) *
                cell.get_ionic_fraction(ION_Ne_n);

    double c6300 = 0.;
    double c9405 = 0.;
    double c6312 = 0.;
    double c33mu = 0.;
    double c19mu = 0.;
    double c3729 = 0.;
    double c3727 = 0.;
    double c7330 = 0.;
    double c4363 = 0.;
    double c5007 = 0.;
    double c52mu = 0.;
    double c5755 = 0.;
    double c6584 = 0.;
    double c4072 = 0.;
    double c6717 = 0.;
    double c6725 = 0.;
    double c3869 = 0.;
    double cniii57 = 0.;
    double cneii12 = 0.;
    double cneiii15 = 0.;
    double cnii122 = 0.;
    double cii2325 = 0.;
    double ciii1908 = 0.;
    double coii7325 = 0.;
    double csiv10 = 0.;
    double c88mu = 0.;
    lines.linestr(cell.get_temperature(), ne, abund, c6300, c9405, c6312, c33mu,
                  c19mu, c3729, c3727, c7330, c4363, c5007, c52mu, c88mu, c5755,
                  c6584, c4072, c6717, c6725, c3869, cniii57, cneii12, cneiii15,
                  cnii122, cii2325, ciii1908, coii7325, csiv10);

    double t4 = cell.get_temperature() * 1.e-4;
    eval.set_emissivity(EMISSIONLINE_HAlpha,
                        ne * nhp * 2.87 * 1.24e-5 * std::pow(t4, -0.938));
    eval.set_emissivity(EMISSIONLINE_HBeta,
                        ne * nhp * 1.24e-5 * std::pow(t4, -0.878));
    eval.set_emissivity(EMISSIONLINE_HII,
                        nhp * ne * 4.9e-7 * std::pow(t4, -0.848));

    double emhpl = 0.;
    double emhmi = 0.;
    double emhepl = 0.;
    double emhemi = 0.;
    bjump(cell.get_temperature(), emhpl, emhmi, emhepl, emhemi);
  }

  return eval;
}

/**
 * @brief Calculate the emissivities for all cell in the given DensityGrid.
 *
 * @param grid DensityGrid to operate on.
 */
void EmissivityCalculator::calculate_emissivities(DensityGrid &grid) {
  // do stuff
}
