// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4;  -*-
//
//  winsorize -- Data cleaning by winsorization
//
//  Copyright (C) 2015           Andreas Alfons and Dirk Eddelbuettel 
//
//  This file is part of winsorize
//
//  winsorize is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  winsorize is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with RcppAPT.  If not, see <http://www.gnu.org/licenses/>.

//  This file originated with robustHD by Andreas Alfons and was subsequently
//  modified for use in winsorize by Dirk Eddelbuettel

/*
 * Author: Andreas Alfons
 *         KU Leuven
 */

#ifndef _robustHD_CORHUBER_H
#define _robustHD_CORHUBER_H

#define ARMA_NO_DEBUG

#include <RcppArmadillo.h>

using namespace arma;

// functions to export to R
// RcppExport SEXP R_corHuberUni(SEXP R_x, SEXP R_y, SEXP R_c);
// RcppExport SEXP R_corHuberAdj(SEXP R_x, SEXP R_y, SEXP R_c);
// RcppExport SEXP R_corHuberBi(SEXP R_x, SEXP R_y, SEXP R_c,
// 		SEXP R_prob, SEXP R_tol);
// RcppExport SEXP R_corMatHuber(SEXP R_x, SEXP R_c, SEXP R_prob, SEXP R_tol);

// functions to be used within C++
double corPearson(const vec& x, const vec& y);
double corHuberBi(const vec& x, const vec& y, const double& c,
                  const double& prob, const double& tol);

#endif
