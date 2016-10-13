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
 * @file testVernerCrossSections.cpp
 *
 * @brief Unit test for the VernerCrossSections class.
 *
 * @author Bert Vandenbroucke (bv7@st-andrews.ac.uk)
 */
#include "Assert.hpp"
#include "ElementNames.hpp"
#include "Error.hpp"
#include "VernerCrossSections.hpp"
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

/**
 * @brief Unit test for the VernerCrossSections class.
 *
 * @param argc Number of command line arguments.
 * @param argv Command line arguments.
 * @return Exit code: 0 on success.
 */
int main(int argc, char **argv) {
  VernerCrossSections cross_sections;

  ifstream file("verner_testdata.txt");
  string line;
  while (getline(file, line)) {
    if (line[0] != '#') {
      stringstream linestream(line);
      unsigned int nz, ne, si;
      double e, s;
      linestream >> nz >> ne >> si >> e >> s;
      assert_values_equal_tol(
          s, cross_sections.get_cross_section_verner(nz, ne, si, e), 1.e-6);
    }
  }

  for (unsigned int i = 0; i < 100; ++i) {
    double nu = 1. + (i + 0.5) * 0.03;
    assert_condition(
        cross_sections.get_cross_section_verner(1, 1, 1, nu * 13.6) ==
        cross_sections.get_cross_section(ELEMENT_H, nu * 13.6));
  }

  return 0;
}
