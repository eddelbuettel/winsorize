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

#include <R.h>
#include "corHuber.h"

using namespace Rcpp;
using namespace arma;


// -------------------
// Pearson correlation
// -------------------

//double corPearson(const vec& x, const vec& y) {
//	const uword n = x.n_elem;
//	// compute means
//	double meanX = 0, meanY = 0;
//	for(uword i = 0; i < n; i++) {
//		meanX += x(i);
//		meanY += y(i);
//	}
//	meanX /= n;
//	meanY /= n;
//	// compute Pearson correlation
//	// covariance and variances are computed up to scaling factor (n-1)
//	double covXY = 0, varX = 0, varY = 0;
//	for(uword i = 0; i < n; i++) {
//		double tmpX = x(i) - meanX, tmpY = y(i) - meanY;  // centered values
//		covXY += tmpX * tmpY;
//		varX += tmpX * tmpX;
//		varY += tmpY * tmpY;
//	}
//	return covXY / (sqrt(varX) * sqrt(varY));
//}
double corPearson(const vec& x, const vec& y) {
	return as_scalar(cor(x, y));	// arma function cor() always returns matrix
}


// ----------------------------------------
// Huber correlation based on winsorization
// ----------------------------------------

// C++ functions for robust correlation based on winsorization assume
// that the data are already robustly standardized

// shrink a single observation
// cm ... winsorization constant for negative values
// cp ... winsorization constant for positive values
double winsorize(const double& x, const double& cm, const double& cp) {
	if(x < cm) {
		return cm;
	} else if(x > cp) {
		return cp;
	} else return x;
}

// robust correlation based on univariate winsorization
// [[Rcpp::export]]
double corHuberUni(const vec& x, const vec& y, const double& c) {
	const double cm = -c;		// negative winsorization constant
	const uword n = x.n_elem;
	vec wx(n), wy(n);
	for(uword i = 0; i < n; i++) {
		wx(i) = winsorize(x(i), cm, c);
		wy(i) = winsorize(y(i), cm, c);
	}
	// call barebones function for Pearson correlation with winsorized data
	return corPearson(wx, wy);
}

// robust correlation based on adjusted univariate winsorization
// [[Rcpp::export]]
double corHuberAdj(const vec& x, const vec& y, const double& c) {
	const uword n = x.n_elem;
	ivec sign(n);
	uword n1 = 0, n2 = 0;
	for(uword i = 0; i < n; i++) {
		double xy = x(i) * y(i);
		if(xy > 0) {
			sign(i) = 1;
			n1 += 1;	// count number of observations in 1. or 3. quadrant
		} else if(xy < 0) {
			sign(i) = -1;
			n2 += 1;	// count number of observations in 2. or 4. quadrant
		} else {
			sign(i) = 0;
			n1 += 1;	// borders are included
			n2 += 1;	// in both counts
		}
	}
	double c1, c2;	// constants for winsorization
	if(n1 < n2) {
		// winsorization with larger constant:  2. and 4. quadrants
		// winsorization with smaller constant: 1. and 3. quadrants
		n1 = n - n2;	// borders are included in major quadrants
		c2 = c;							// larger winsorization constant
		c1 = sqrt(n1/(double)n2) * c2;	// smaller winsorization constant
	} else {
		// winsorization with larger constant:  1. and 3. quadrants
		// winsorization with smaller constant: 2. and 4. quadrants
		// if there are the same number of observations in 1./3. and 2./4.
		// quadrants, 1./3. are assumed to be the major quadrants
		n2 = n - n1;	// borders are included in major quadrants
		c1 = c;							// larger winsorization constant
		c2 = sqrt(n2/(double)n1) * c1;	// smaller winsorization constant
	}
	// winsorize data
	const double cm = -c, cm1 = -c1, cm2 = -c2;
	vec wx(n), wy(n);
	for(uword i = 0; i < n; i++) {
		if(sign(i) == 1) {
			wx(i) = winsorize(x(i), cm1, c1);
			wy(i) = winsorize(y(i), cm1, c1);
		} else if(sign(i) == -1) {
			wx(i) = winsorize(x(i), cm2, c2);
			wy(i) = winsorize(y(i), cm2, c2);
		} else {
			wx(i) = winsorize(x(i), cm, c);
			wy(i) = winsorize(y(i), cm, c);
		}
	}
	// call barebones function for Pearson correlation with winsorized data
	return corPearson(wx, wy);
}


// robust correlation based on bivariate winsorization
// [[Rcpp::export]]
double corHuberBi(const vec& x, const vec& y, const double& c,
                  const double& prob, const double& tol) {
	// compute the initial correlation matrix with adjusted winsorization
	double r0 = corHuberAdj(x, y, c);
	// C++ isnan() is not portable and gives error on Windows systems
	// use R macro ISNAN() instead
	if(ISNAN(r0) || (1 - abs(r0) < tol)) {
		// points almost on a line, leads to computationally singular system
		return r0;
	}
	// construct correlation matrix
	mat R0 = eye<mat>(2, 2);	// initialize identity matrix
	R0(1,0) = r0;				// set off-diagonal elements to
	R0(0,1) = r0;				// initial correlation estimate
	// compute Mahalanobis distances
	mat xy = join_rows(x, y);			// put x and y in a matrix
	mat invR0 = inv(R0);				// inverse correlation matrix
	vec md = sum((xy * invR0) % xy, 1);	// squared Mahalanobis distances
	// shrink observations with too large distances
	// compute the quantile of the chi-squared distribution with 2 degrees of
	// freedom corresponding to probability 'prob'
	// note that (1 - prob) needs to be used in the call to Rf_qchisq()
	double d = Rf_qchisq(1 - prob, 2, 0, 0);
	// shrink observations with too large distances
	for(uword i = 0; i < xy.n_rows; i++) {
		if(md(i) > d){
			xy.row(i) *= sqrt(d / md(i));
		}
	}
	// call barebones function for Pearson correlation with winsorized data
	return corPearson(xy.col(0), xy.col(1));
}


// robust correlation matrix based on bivariate winsorization
// [[Rcpp::export]]
mat corMatHuber(const mat& x, const double& c,
		const double& prob, const double& tol) {
	const uword p = x.n_cols;
	mat R = eye<mat>(p, p);	// initialize identity matrix
	// compute off-diagonal elements
	for(uword j = 0; j < p; j++) {
		vec xj = x.unsafe_col(j);
		for(uword i = j+1; i < p; i++) {
			R(i, j) = corHuberBi(x.unsafe_col(i), xj, c, prob, tol);
			R(j, i) = R(i, j);
		}
	}
	// return robust correlation matrix
	return R;
}

