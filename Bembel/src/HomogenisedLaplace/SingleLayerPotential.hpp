// This file is part of Bembel, the higher order C++ boundary element library.
//
// Copyright (C) 2022 see <http://www.bembel.eu>
//
// It was written as part of a cooperation of J. Doelz, H. Harbrecht, S. Kurz,
// M. Multerer, S. Schoeps, and F. Wolf at Technische Universitaet Darmstadt,
// Universitaet Basel, and Universita della Svizzera italiana, Lugano. This
// source code is subject to the GNU General Public License version 3 and
// provided WITHOUT ANY WARRANTY, see <http://www.bembel.eu> for further
// information.

#ifndef BEMBEL_SRC_HOMOGENISEDLAPLACE_SINGLELAYERPOTENTIAL_HPP_
#define BEMBEL_SRC_HOMOGENISEDLAPLACE_SINGLELAYERPOTENTIAL_HPP_

namespace Bembel {
// forward declaration of class HomogenisedLaplaceSingleLayerPotential
// in order to define traits
template<typename LinOp>
class HomogenisedLaplaceSingleLayerPotential;

template<typename LinOp>
struct PotentialTraits<HomogenisedLaplaceSingleLayerPotential<LinOp>> {
  typedef Eigen::VectorXd::Scalar Scalar;
  static constexpr int OutputSpaceDimension = 1;
};

/**
 * \ingroup HomogenisedLaplace
 */
template<typename LinOp>
class HomogenisedLaplaceSingleLayerPotential : public PotentialBase<
    HomogenisedLaplaceSingleLayerPotential<LinOp>, LinOp> {
  // implementation of the kernel evaluation, which may be based on the
  // information available from the superSpace

 public:
  /**
     * \brief Constructs an object initialising the coefficients and the degree
     *  via the static variable HomogenisedLaplaceSingleLayerOperator::precision.
     */
  HomogenisedLaplaceSingleLayerPotential() {
    this->deg = getDegree(
        HomogenisedLaplaceSingleLayerOperator::getPrecision());
    this->cs = getCoefficients(
        HomogenisedLaplaceSingleLayerOperator::getPrecision());
  }
  Eigen::Matrix<
      typename PotentialReturnScalar<
          typename LinearOperatorTraits<LinOp>::Scalar,
            double>::Scalar, 1, 1> evaluateIntegrand_impl(
      const FunctionEvaluator<LinOp> &fun_ev, const ElementTreeNode &element,
      const Eigen::Vector3d &point, const SurfacePoint &p) const {
    // get evaluation points on unit square
    auto s = p.segment < 2 > (0);

    // get quadrature weights
    auto ws = p(2);

    // get points on geometry and tangential derivatives
    auto x_f = p.segment < 3 > (3);
    auto x_f_dx = p.segment < 3 > (6);
    auto x_f_dy = p.segment < 3 > (9);

    // compute surface measures from tangential derivatives
    auto x_kappa = x_f_dx.cross(x_f_dy).norm();

    // evaluate kernel
    auto kernel = evaluateKernel(point, x_f);

    // assemble Galerkin solution
    auto cauchy_value = fun_ev.evaluate(element, p);

    // integrand without basis functions
    auto integrand = kernel * cauchy_value * x_kappa * ws;

    return integrand;
  }

  /**
   * \brief Fundamental solution of the homogenised Laplace problem
   */
  double evaluateKernel(const Eigen::Vector3d &x,
      const Eigen::Vector3d &y) const {
    return k_mod(x - y)
        + evaluate_solid_sphericals(x - y, this->cs, this->deg, false);
  }

 private:
  /** The degree of the spherical harmonics expansion */
  unsigned int deg;
  /** The coefficients of the spherical harmonics expansion */
  Eigen::VectorXd cs;
};

}  // namespace Bembel

#endif  // BEMBEL_SRC_HOMOGENISEDLAPLACE_SINGLELAYERPOTENTIAL_HPP_
