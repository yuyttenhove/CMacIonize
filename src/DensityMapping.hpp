/*******************************************************************************
 * Copyright (C) 2017 Bert Vandenbroucke (bert.vandenbroucke@gmail.com)
 *               2018 Maya Petkova (maya.petkova@uni-heidelberg.de)
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
 * @file DensityMapping.hpp
 *
 * @brief Density mapping functions.
 *
 * Based on the expression of Petkova et al. 2018.
 *
 * @author Maya Petkova (maya.petkova@uni-heidelberg.de)
 */
#ifndef DENSITYMAPPING_HPP
#define DENSITYMAPPING_HPP

#include <vector>

/**
 * @brief Density mapping functions.
 *
 *  Based on the expression of Petkova et al. 2018.
 */
class DensityMapping {
private:
  /*! @brief Grid of pre-computed cell densities. */
  std::vector< std::vector< std::vector< double > > > _density_values;

public:
  /**
   * @brief Initialize the pre-computed array of density values.
   */
  void gridding() {
    double phi, r0, R_0, h, mu0;
    int i, j, k, n, nr1, nr2;
    double rl, mul, cphil;

    h = 1.0;
    n = 150;
    nr1 = 50;
    nr2 = 199;
    rl = 0.1;
    mul = 0.98;
    cphil = 0.98;

    _density_values.resize(nr1 + nr2 + 2);
    for (i = 0; i <= nr1 + nr2 + 1; i++) {
      _density_values[i].resize(2 * n + 1);
      for (j = 0; j <= 2 * n; j++) {
        _density_values[i][j].resize(2 * n + 1, 0.);
      }
    }

    for (i = 0; i < nr1; i++) {
      r0 = (rl / nr1) * (i + 1) * h;
      for (j = 0; j < n; j++) {
        mu0 = (mul / (n - 1)) * j;
        R_0 = r0 * (sqrt(1.0 - mu0 * mu0)) / mu0;
        for (k = 0; k < n; k++) {
          phi = acos((cphil / (n - 1)) * k);
          _density_values[i + 1][j][k] = full_integral(phi, r0, R_0, h);
        }

        for (k = 1; k <= n; k++) {
          phi = acos(cphil + ((1.0 - cphil) / n) * k);
          _density_values[i + 1][j][n + k - 1] = full_integral(phi, r0, R_0, h);
        }
      }

      for (j = 1; j <= n; j++) {
        mu0 = mul + ((1.0 - mul) / n) * j;
        R_0 = r0 * (sqrt(1.0 - mu0 * mu0)) / mu0;
        for (k = 0; k < n; k++) {
          phi = acos((cphil / (n - 1)) * k);
          _density_values[i + 1][n + j - 1][k] = full_integral(phi, r0, R_0, h);
        }

        for (k = 1; k <= n; k++) {
          phi = acos(cphil + ((1.0 - cphil) / n) * k);
          _density_values[i + 1][n + j - 1][n + k - 1] =
              full_integral(phi, r0, R_0, h);
        }
      }
    }

    for (i = 1; i <= nr2; i++) {
      r0 = rl + ((2.0 - rl) / nr2) * i * h;
      for (j = 0; j < n; j++) {
        mu0 = (mul / (n - 1)) * j;
        R_0 = r0 * (sqrt(1.0 - mu0 * mu0)) / mu0;

        for (k = 0; k < n; k++) {
          phi = acos((cphil / (n - 1)) * k);
          _density_values[nr1 + i][j][k] = full_integral(phi, r0, R_0, h);
        }

        for (k = 1; k <= n; k++) {
          phi = acos(cphil + ((1.0 - cphil) / n) * k);
          _density_values[nr1 + i][j][n + k - 1] =
              full_integral(phi, r0, R_0, h);
        }
      }

      for (j = 1; j <= n; j++) {
        mu0 = mul + ((1.0 - mul) / n) * j;
        R_0 = r0 * (sqrt(1.0 - mu0 * mu0)) / mu0;

        for (k = 0; k < n; k++) {
          phi = acos((cphil / (n - 1)) * k);
          _density_values[nr1 + i][n + j - 1][k] =
              full_integral(phi, r0, R_0, h);
        }

        for (k = 1; k <= n; k++) {
          phi = acos(cphil + ((1.0 - cphil) / n) * k);
          _density_values[nr1 + i][n + j - 1][n + k - 1] =
              full_integral(phi, r0, R_0, h);
        }
      }
    }

    i = nr1 + nr2;
    for (j = 0; j < 2 * n; j++) {
      for (k = 0; k < 2 * n; k++) {
        _density_values[i + 1][j][k] = _density_values[i][j][k];
      }
    }

    /*  for(i=0;i<10;i++) {
      for(j=0;j<10;j++) {
        for(k=0;k<10;k++) {
          printf("%g, ",_density_values[i][j][k]);
        }
        printf("\n");
      }
      printf("\n");
      }*/
  }

  /**
   * @brief Function that gives the 3D integral of the kernel of
   * a particle for a given vertex of a cell face, interpolated from a
   * pre-computed grid.
   *
   * @param phi Azimuthal angle of the vertex.
   * @param r0_old Distance from the particle to the face of the cell (in m).
   * @param R_0_old Distance from the orthogonal projection of the particle
   * onto the face of the cell to a side of the face (containing the vertex; in
   * m).
   * @param h_old The kernel smoothing length of the particle (in m).
   * @return The integral of the kernel for the given vertex.
   */

  double gridded_integral(double phi, double r0_old, double R_0_old,
                          double h_old) const {

    double r0, R_0, cphi, mu0, h;
    int i, j, k;
    double fx1, fx2, fx3, fx4, fy1, fy2, fz, frac;
    const int nr01 = 50;
    const int nr02 = 199;
    const int nR_01 = 150;
    const int nR_02 = 150;
    const int nphi1 = 150;
    const int nphi2 = 150;
    const double rl = 0.1;
    const double mul = 0.98;
    const double cphil = 0.98;

    h = 1.0;
    r0 = r0_old / h_old;
    R_0 = R_0_old / h_old;
    cphi = cos(phi);

    if (r0 < 0.0) {
      printf("Error: r0 < 0: %g %g\n", r0, r0_old);
      r0 = 0.0;
    }

    if (R_0 < 0.0) {
      printf("Error: R_0 < 0: %g %g\n", R_0, R_0_old);
      R_0 = 0.0;
    }

    if (r0 == 0.0)
      return 0.0;
    if (R_0 == 0.0)
      return 0.0;
    if (phi == 0.0)
      return 0.0;

    mu0 = r0 / sqrt(r0 * r0 + R_0 * R_0);

    if (r0 > 2.0)
      r0 = 2.0;

    if (r0 < rl) {
      i = int(r0 / (rl / nr01));
    } else {
      i = nr01 + int((r0 - rl) / ((2.0 - rl) * h / nr02));
    }

    if (mu0 < mul) {
      j = int(mu0 / (mul / (nR_01 - 1)));
    } else {
      j = nR_01 - 1 + int((mu0 - mul) / ((1.0 - mul) / nR_02));
    }

    if (cphi < cphil) {
      k = int(cphi / (cphil / (nphi1 - 1)));
    } else {
      k = nphi1 - 1 + int((cphi - cphil) / ((1.0 - cphil) / nphi2));
    }

    if (i < 0 || i > nr01 + nr02 || j < 0 || j > nR_01 + nR_02 - 1 || k < 0 ||
        k > nphi1 + nphi2)
      printf("i, j, k: %d %d %d \n", i, j, k);

    if (r0 < rl) {
      frac = (r0 - (rl / nr01) * i * h) / (rl / nr01 * h);
    } else {
      frac = (r0 - rl - ((2.0 - rl) / nr02) * (i - nr01) * h) /
             ((2.0 - rl) / nr02 * h);
    }
    fx1 = frac * DensityMapping::get_gridded_density_value(i + 1, j, k) +
          (1 - frac) * DensityMapping::get_gridded_density_value(i, j, k);
    fx2 = frac * DensityMapping::get_gridded_density_value(i + 1, j + 1, k) +
          (1 - frac) * DensityMapping::get_gridded_density_value(i, j + 1, k);
    fx3 = frac * DensityMapping::get_gridded_density_value(i + 1, j, k + 1) +
          (1 - frac) * DensityMapping::get_gridded_density_value(i, j, k + 1);
    fx4 =
        frac * DensityMapping::get_gridded_density_value(i + 1, j + 1, k + 1) +
        (1 - frac) * DensityMapping::get_gridded_density_value(i, j + 1, k + 1);

    /*if(j < nR_0/2-1) {
      frac = (mu0 - (1.0/(nR_0/2))*j)/(1.0/(nR_0/2));
    } else {
      frac = (mu0 - (nR_0/2-1)*(1.0/(nR_0/2)) -
    (1.0/(nR_0/2)/(nR_0/2))*(j-(nR_0/2-1)))/(1.0/(nR_0/2)/(nR_0/2));
      }*/

    if (mu0 < mul) {
      frac = (mu0 - mul / (nR_01 - 1) * j) / (mul / (nR_01 - 1));
    } else {
      frac = (mu0 - mul - (1.0 - mul) / nR_02 * (j - nR_01 + 1)) /
             ((1.0 - mul) / nR_02);
    }
    fy1 = frac * fx2 + (1 - frac) * fx1;
    fy2 = frac * fx4 + (1 - frac) * fx3;

    /*if(k < nphi/2-1) {
      frac = (cphi - (1.0/(nphi/2))*k)/(1.0/(nphi/2));
    } else {
      frac = (cphi - (nphi/2-1)*(1.0/(nphi/2)) -
    (1.0/(nphi/2)/(nphi/2))*(k-(nphi/2-1)))/(1.0/(nphi/2)/(nphi/2));
      }*/

    if (cphi < cphil) {
      frac = (cphi - cphil / (nphi1 - 1) * k) / (cphil / (nphi1 - 1));
    } else {
      frac = (cphi - cphil - (1.0 - cphil) / nphi2 * (k - nphi1 + 1)) /
             ((1.0 - cphil) / nphi2);
    }
    fz = frac * fy2 + (1 - frac) * fy1;

    if ((j == (nR_01 + nR_02 - 1)) || (k == (nphi1 + nphi2 - 1)))
      fz = 0.0;

    return fz;
  }

  /**
   * @brief Function that gives the 3D integral of the kernel of
   * a particle for a given vertex of a cell face.
   *
   * @param phi Azimuthal angle of the vertex.
   * @param r0 Distance from the particle to the face of the cell (in m).
   * @param R_0 Distance from the orthogonal projection of the particle
   * onto the face of the cell to a side of the face (containing the vertex; in
   * m).
   * @param h The kernel smoothing length of the particle (in m).
   * @return The integral of the kernel for the given vertex.
   */

  double full_integral(double phi, double r0, double R_0, double h) const {

    double B1, B2, B3, mu, a, logs, u;
    double full_int;
    double a2, cosp, cosp2, r03, r0h2, r0h3, r0h_2, r0h_3, tanp;
    double r2, R, linedist2, phi1, phi2, h2;
    double I0, I1, I_1, I_2, I_3, I_4, I_5;
    double D2, D3;

    B1 = 0.0;
    B2 = 0.0;
    B3 = 0.0;

    if (r0 == 0.0)
      return 0.0;
    if (R_0 == 0.0)
      return 0.0;
    if (phi == 0.0)
      return 0.0;

    h2 = h * h;
    r03 = r0 * r0 * r0;
    r0h2 = r0 / h * r0 / h;
    r0h3 = r0h2 * r0 / h;
    r0h_2 = h / r0 * h / r0;
    r0h_3 = r0h_2 * h / r0;

    // Setting up the B1, B2, B3 constants of integration.

    if (r0 >= 2.0 * h) {
      B3 = h2 * h / 4.;
    } else if (r0 > h) {
      B3 = r03 / 4. *
           (-4. / 3. + (r0 / h) - 0.3 * r0h2 + 1. / 30. * r0h3 -
            1. / 15. * r0h_3 + 8. / 5. * r0h_2);
      B2 = r03 / 4. *
           (-4. / 3. + (r0 / h) - 0.3 * r0h2 + 1. / 30. * r0h3 -
            1. / 15. * r0h_3);
    } else {
      B3 = r03 / 4. * (-2. / 3. + 0.3 * r0h2 - 0.1 * r0h3 + 7. / 5. * r0h_2);
      B2 = r03 / 4. * (-2. / 3. + 0.3 * r0h2 - 0.1 * r0h3 - 1. / 5. * r0h_2);
      B1 = r03 / 4. * (-2. / 3. + 0.3 * r0h2 - 0.1 * r0h3);
    }

    a = R_0 / r0;
    a2 = a * a;

    linedist2 = r0 * r0 + R_0 * R_0;
    R = R_0 / cos(phi);
    r2 = r0 * r0 + R * R;

    full_int = 0.0;
    D2 = 0.0;
    D3 = 0.0;

    if (linedist2 <= h2) {
      ////// phi1 business /////
      phi1 = acos(R_0 / sqrt(h * h - r0 * r0));

      cosp = cos(phi1);
      cosp2 = cosp * cosp;
      mu = cosp / a / sqrt(1. + cosp2 / a2);

      tanp = tan(phi1);

      I0 = phi1;
      I_2 = phi1 + a2 * tanp;
      I_4 = phi1 + 2 * a2 * tanp + 1. / 3. * a2 * a2 * tanp * (2. + 1. / cosp2);

      u = sin(phi1) * sqrt(1 - mu * mu);
      logs = log(1 + u) - log(1 - u);
      I1 = atan(u / a);

      I_1 = a / 2. * logs + I1;
      I_3 = I_1 + a * (1. + a2) / 4. * (2 * u / (1 - u * u) + logs);
      I_5 = I_3 + a * (1. + a2) * (1. + a2) / 16. *
                      ((10 * u - 6 * u * u * u) / (1 - u * u) / (1 - u * u) +
                       3. * logs);

      D2 = -1. / 6. * I_2 + 0.25 * (r0 / h) * I_3 - 0.15 * r0h2 * I_4 +
           1. / 30. * r0h3 * I_5 - 1. / 60. * r0h_3 * I1 + (B1 - B2) / r03 * I0;

      ////// phi2 business /////
      phi2 = acos(R_0 / sqrt(4.0 * h * h - r0 * r0));

      cosp = cos(phi2);
      cosp2 = cosp * cosp;
      mu = cosp / a / sqrt(1. + cosp2 / a2);

      tanp = tan(phi2);

      I0 = phi2;
      I_2 = phi2 + a2 * tanp;
      I_4 = phi2 + 2 * a2 * tanp + 1. / 3. * a2 * a2 * tanp * (2. + 1. / cosp2);

      u = sin(phi2) * sqrt(1 - mu * mu);
      logs = log(1 + u) - log(1 - u);
      I1 = atan(u / a);

      I_1 = a / 2. * logs + I1;
      I_3 = I_1 + a * (1. + a2) / 4. * (2 * u / (1 - u * u) + logs);
      I_5 = I_3 + a * (1. + a2) * (1. + a2) / 16. *
                      ((10 * u - 6 * u * u * u) / (1 - u * u) / (1 - u * u) +
                       3. * logs);

      D3 = 1. / 3. * I_2 - 0.25 * (r0 / h) * I_3 + 3. / 40. * r0h2 * I_4 -
           1. / 120. * r0h3 * I_5 + 4. / 15. * r0h_3 * I1 +
           (B2 - B3) / r03 * I0 + D2;
    } else if (linedist2 <= 4.0 * h2) {
      ////// phi2 business /////
      phi2 = acos(R_0 / sqrt(4.0 * h * h - r0 * r0));

      cosp = cos(phi2);
      cosp2 = cosp * cosp;
      mu = cosp / a / sqrt(1. + cosp2 / a2);

      tanp = tan(phi2);

      I0 = phi2;
      I_2 = phi2 + a2 * tanp;
      I_4 = phi2 + 2 * a2 * tanp + 1. / 3. * a2 * a2 * tanp * (2. + 1. / cosp2);

      u = sin(phi2) * sqrt(1 - mu * mu);
      logs = log(1 + u) - log(1 - u);
      I1 = atan(u / a);

      I_1 = a / 2. * logs + I1;
      I_3 = I_1 + a * (1. + a2) / 4. * (2 * u / (1 - u * u) + logs);
      I_5 = I_3 + a * (1. + a2) * (1. + a2) / 16. *
                      ((10 * u - 6 * u * u * u) / (1 - u * u) / (1 - u * u) +
                       3. * logs);

      D3 = 1. / 3. * I_2 - 0.25 * (r0 / h) * I_3 + 3. / 40. * r0h2 * I_4 -
           1. / 120. * r0h3 * I_5 + 4. / 15. * r0h_3 * I1 +
           (B2 - B3) / r03 * I0 + D2;
    }

    //////////////////////////////////
    // Calculating I_n expressions. //
    //////////////////////////////////

    cosp = cos(phi);
    cosp2 = cosp * cosp;
    mu = cosp / a / sqrt(1. + cosp2 / a2);

    tanp = tan(phi);

    I0 = phi;
    I_2 = phi + a2 * tanp;
    I_4 = phi + 2 * a2 * tanp + 1. / 3. * a2 * a2 * tanp * (2. + 1. / cosp2);

    u = sin(phi) * sqrt(1 - mu * mu);
    logs = log(1 + u) - log(1 - u);
    I1 = atan(u / a);

    I_1 = a / 2. * logs + I1;
    I_3 = I_1 + a * (1. + a2) / 4. * (2 * u / (1 - u * u) + logs);
    I_5 = I_3 + a * (1. + a2) * (1. + a2) / 16. *
                    ((10 * u - 6 * u * u * u) / (1 - u * u) / (1 - u * u) +
                     3. * logs);

    // Calculating the integral expression.

    if (r2 < h2) {
      full_int = r0h3 / M_PI *
                 (1. / 6. * I_2 - 3. / 40. * r0h2 * I_4 +
                  1. / 40. * r0h3 * I_5 + B1 / r03 * I0);
    } else if (r2 < 4.0 * h2) {
      full_int = r0h3 / M_PI *
                 (0.25 * (4. / 3. * I_2 - (r0 / h) * I_3 + 0.3 * r0h2 * I_4 -
                          1. / 30. * r0h3 * I_5 + 1. / 15. * r0h_3 * I1) +
                  B2 / r03 * I0 + D2);
    } else {
      full_int = r0h3 / M_PI * (-0.25 * r0h_3 * I1 + B3 / r03 * I0 + D3);
    }

    return full_int;
  }

  /**
   * @brief Function that calculates the mass contribution of a particle
   * towards the total mass of a cell.
   *
   * @param cell Geometrical information about the cell.
   * @param particle The particle position (in m).
   * @param h The kernel smoothing length of the particle (in m).
   * @return The mass contribution of the particle to the cell.
   */

  double mass_contribution(const Cell &cell, CoordinateVector<> particle,
                           double h) const {

    double M, Msum;

    Msum = 0.;
    M = 0.;

    std::vector< Face > face_vector = cell.get_faces();

    // Loop over each face of a cell.
    for (size_t i = 0; i < face_vector.size(); i++) {

      CoordinateVector<> vert_position1;
      CoordinateVector<> projected_particle;
      double r0 = 0.;
      double ar0 = 0.;
      double s2 = 0.;

      // Loop over the vertices of each face.
      for (Face::Vertices j = face_vector[i].first_vertex();
           j != face_vector[i].last_vertex(); ++j) {
        if (j == face_vector[i].first_vertex()) { // Calculating the distance
                                                  // from particle to each face
                                                  // of a cell
          // http://mathinsight.org/distance_point_plane
          // http://mathinsight.org/forming_planes
          Face::Vertices j_twin = j;
          vert_position1 = j_twin.get_position();
          CoordinateVector<> vert_position2 = (++j_twin).get_position();
          CoordinateVector<> vert_position3 = (++j_twin).get_position();

          const double A = (vert_position2[1] - vert_position1[1]) *
                               (vert_position3[2] - vert_position1[2]) -
                           (vert_position3[1] - vert_position1[1]) *
                               (vert_position2[2] - vert_position1[2]);
          const double B = (vert_position2[2] - vert_position1[2]) *
                               (vert_position3[0] - vert_position1[0]) -
                           (vert_position3[2] - vert_position1[2]) *
                               (vert_position2[0] - vert_position1[0]);
          const double C = (vert_position2[0] - vert_position1[0]) *
                               (vert_position3[1] - vert_position1[1]) -
                           (vert_position3[0] - vert_position1[0]) *
                               (vert_position2[1] - vert_position1[1]);
          const double D = -A * vert_position1[0] - B * vert_position1[1] -
                           C * vert_position1[2];

          const double norm = sqrt(A * A + B * B + C * C);
          r0 = (A * particle[0] + B * particle[1] + C * particle[2] + D) / norm;
          ar0 = fabs(r0);

          // Calculate of the orthogonal projection of the particle position
          // onto the face.
          projected_particle[0] = particle[0] - r0 * A / norm;
          projected_particle[1] = particle[1] - r0 * B / norm;
          projected_particle[2] = particle[2] - r0 * C / norm;

          // s2 contains information about the orientation of the face vertices.
          s2 = vert_position1[0] * (vert_position2[1] * vert_position3[2] -
                                    vert_position2[2] * vert_position3[1]) +
               vert_position1[1] * (vert_position2[2] * vert_position3[0] -
                                    vert_position2[0] * vert_position3[2]) +
               vert_position1[2] * (vert_position2[0] * vert_position3[1] -
                                    vert_position2[1] * vert_position3[0]);
        }

        Face::Vertices j_twin = j;
        const CoordinateVector<> vert_position2 = j_twin.get_position();
        CoordinateVector<> vert_position3;

        if (++j_twin == face_vector[i].last_vertex()) {
          vert_position3 = vert_position1;
        } else {
          vert_position3 = j_twin.get_position();
        }

        const double r23 = (vert_position2 - vert_position3).norm();
        const double r12 = (projected_particle - vert_position2).norm();
        const double r13 = (projected_particle - vert_position3).norm();
        const double cosa = ((vert_position3[0] - vert_position2[0]) *
                                 (projected_particle[0] - vert_position2[0]) +
                             (vert_position3[1] - vert_position2[1]) *
                                 (projected_particle[1] - vert_position2[1]) +
                             (vert_position3[2] - vert_position2[2]) *
                                 (projected_particle[2] - vert_position2[2])) /
                            r12 / r23;

        double R_0 = 0.;
        double phi1 = 0.;
        double phi2 = 0.;

        if (fabs(cosa) < 1.0) {
          R_0 = r12 * sqrt(1 - cosa * cosa);
        } else {
          if (fabs(cosa) - 1.0 < 0.00001) {
            R_0 = 0.0;
          } else {
            printf("Error: cosa > 1: %g\n", cosa);
          }
        }

        const double s1 =
            projected_particle[0] * (vert_position2[1] * vert_position3[2] -
                                     vert_position2[2] * vert_position3[1]) +
            projected_particle[1] * (vert_position2[2] * vert_position3[0] -
                                     vert_position2[0] * vert_position3[2]) +
            projected_particle[2] * (vert_position2[0] * vert_position3[1] -
                                     vert_position2[1] * vert_position3[0]);

        if (R_0 < r12) {
          phi1 = acos(R_0 / r12);
        } else {
          if ((R_0 - r12) / h < 0.00001) {
            phi1 = 0.0;
          } else {
            printf("Error: R0 > r12: %g\n", R_0 - r12);
          }
        }
        if (R_0 < r13) {
          phi2 = acos(R_0 / r13);
        } else {
          if ((R_0 - r13) / h < 0.00001) {
            phi2 = 0.0;
          } else {
            printf("Error: R0 > r13: %g\n", R_0 - r13);
          }
        }

        // Find out if the vertex integral will contribute positively or
        // negatively to the cell mass.
        if (s1 * s2 * r0 <= 0) {
          M = -1.;
        } else {
          M = 1.;
        }

        if (ar0 < 0.0)
          printf("Wrong from mass_contribution: r0 = %g \n", ar0);
        if (R_0 < 0.0)
          printf("Wrong from mass_contribution: R_0 = %g \n", R_0);

        // Calculate the vertex integral.
        const bool is_pre_computed = true;
        if ((r12 * sin(phi1) >= r23) || (r13 * sin(phi2) >= r23)) {
          if (phi1 >= phi2) {
            if (is_pre_computed) {
              M = M * (gridded_integral(phi1, ar0, R_0, h) -
                       gridded_integral(phi2, ar0, R_0, h));
            } else {
              M = M * (full_integral(phi1, ar0, R_0, h) -
                       full_integral(phi2, ar0, R_0, h));
            }
          } else {
            if (is_pre_computed) {
              M = M * (gridded_integral(phi2, ar0, R_0, h) -
                       gridded_integral(phi1, ar0, R_0, h));
            } else {
              M = M * (full_integral(phi2, ar0, R_0, h) -
                       full_integral(phi1, ar0, R_0, h));
            }
          }
        } else {
          if (is_pre_computed) {
            M = M * (gridded_integral(phi1, ar0, R_0, h) +
                     gridded_integral(phi2, ar0, R_0, h));
          } else {
            M = M * (full_integral(phi1, ar0, R_0, h) +
                     full_integral(phi2, ar0, R_0, h));
          }
        }
        Msum = Msum + M;
      }
    }

    return Msum;
  }

  /**
   * @brief Get the gridded density value for the given indices.
   *
   * @param i First index.
   * @param j Second index.
   * @param k Third index.
   * @return Corresponding density value.
   */
  double get_gridded_density_value(int i, int j, int k) const {
    return _density_values[i][j][k];
  }
};

#endif // DENSITYMAPPING_HPP
