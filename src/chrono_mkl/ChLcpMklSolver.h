//
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2010 Alessandro Tasora
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file at the top level of the distribution
// and at http://projectchrono.org/license-chrono.txt.
//

#ifndef CHMKLSOLVER_H
#define CHMKLSOLVER_H

///////////////////////////////////////////////////
//
//   ChLcpMklSolver.h
//
//   Use this header if you want to exploit Intel�
//	 MKL Library
//   from Chrono::Engine programs.
//
//   HEADER file for CHRONO,
//  Multibody dynamics engine
//
// ------------------------------------------------
//             www.deltaknowledge.com
// ------------------------------------------------
///////////////////////////////////////////////////



#include "chrono_mkl/ChApiMkl.h"
#include "core/ChMatrix.h"
#include "lcp/ChLcpSystemDescriptor.h"
#include "lcp/ChLcpSolver.h"
#include "chrono_mkl/ChMklEngine.h"

//#include <process.h>



namespace chrono {


    /// Class that wraps the Intel MKL 'PARDISO' parallel direct solver.
    /// It can solve linear systems. It cannot solve VI and complementarity problems.
    
   class ChApiMkl ChLcpMklSolver : public ChLcpSolver {

   private:
	   long int solver_call = 0;
	   ChCSR3Matrix matCSR3 = ChCSR3Matrix(1,1,1);
	   ChMatrixDynamic<double> rhs;
	   ChMatrixDynamic<double> sol;
	   ChMatrixDynamic<double> res;
	   double res_norm = 0;
	   ChMklEngine mkl_engine{ 1, 11 };

	   bool size_lock = true;
	   bool sparsity_pattern_lock = false;
	   int n = 0;

   public:

	   ChLcpMklSolver(){};
	   virtual ~ChLcpMklSolver(){};
	   
	   ChMklEngine& GetMklEngine(){ return mkl_engine; }
	   ChCSR3Matrix& GetMatrix(){ return matCSR3; }

	   void SetProblemSizeLock(bool on_off){ size_lock = on_off; };
	   void SetSparsityPatternLock(bool on_off){ sparsity_pattern_lock = on_off; };

        /// Solve using the MKL Pardiso sparse direct solver
		virtual double Solve(ChLcpSystemDescriptor& sysd); ///< system description with constraints and variables

		
		
	   


    };

}  // END_OF_NAMESPACE____

#endif
