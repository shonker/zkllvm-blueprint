#include <stdlib.h>
#include <iostream>

#include <nil/crypto3/zk/snark/blueprint.hpp>

#include <nil/crypto3/algebra/curves/bls12.hpp>
#include <nil/crypto3/algebra/fields/bls12/base_field.hpp>
#include <nil/crypto3/algebra/fields/bls12/scalar_field.hpp>
#include <nil/crypto3/algebra/fields/arithmetic_params/bls12.hpp>
#include <nil/crypto3/algebra/curves/params/multiexp/bls12.hpp>
#include <nil/crypto3/algebra/curves/params/wnaf/bls12.hpp>

#include <nil/crypto3/algebra/curves/alt_bn128.hpp>
#include <nil/crypto3/algebra/fields/alt_bn128/base_field.hpp>
#include <nil/crypto3/algebra/fields/alt_bn128/scalar_field.hpp>
#include <nil/crypto3/algebra/fields/arithmetic_params/alt_bn128.hpp>
#include <nil/crypto3/algebra/curves/params/multiexp/alt_bn128.hpp>
#include <nil/crypto3/algebra/curves/params/wnaf/alt_bn128.hpp>

#include <nil/crypto3/algebra/fields/detail/element/fp.hpp>
#include <nil/crypto3/algebra/fields/detail/element/fp2.hpp>

#include <nil/crypto3/zk/snark/components/curves/edwards/element_g1.hpp>

#include <nil/crypto3/algebra/random_element.hpp>

#include <nil/crypto3/zk/snark/algorithms/generate.hpp>
#include <nil/crypto3/zk/snark/algorithms/verify.hpp>
#include <nil/crypto3/zk/snark/algorithms/prove.hpp>

#include <nil/crypto3/zk/snark/schemes/ppzksnark/r1cs_gg_ppzksnark.hpp>

using namespace nil::crypto3::zk::snark;
using namespace nil::crypto3::algebra;

template<typename FieldParams>
void print_field_element(typename fields::detail::element_fp<FieldParams> e) {
    std::cout << e.data << std::endl;
}

template<typename FpCurveGroupElement>
void print_fp_curve_group_element(FpCurveGroupElement e) {
    std::cout << e.X.data << " " << e.Y.data << " " << e.Z.data << std::endl;
}

template<typename CurveType>
void verify_component(blueprint<typename CurveType::scalar_field_type> bp){
    using field_type = typename CurveType::scalar_field_type;
    using curve_type = CurveType;

    const r1cs_constraint_system<field_type> constraint_system = bp.get_constraint_system();

    const typename r1cs_gg_ppzksnark<curve_type>::keypair_type keypair = generate<r1cs_gg_ppzksnark<curve_type>>(constraint_system);

    const typename r1cs_gg_ppzksnark<curve_type>::proof_type proof = prove<r1cs_gg_ppzksnark<curve_type>>(keypair.first, bp.primary_input(), bp.auxiliary_input());

    bool verified = verify<r1cs_gg_ppzksnark<curve_type>>(keypair.second, bp.primary_input(), proof);

    std::cout << "Number of R1CS constraints: " << constraint_system.num_constraints() << std::endl;
    std::cout << "Verification status: " << verified << std::endl;

    assert(verified);
}

template <typename CurveType>
blueprint<typename CurveType::scalar_field_type> addition_example(
    typename CurveType::pairing::chained_curve_type::g1_type::value_type p1,
    typename CurveType::pairing::chained_curve_type::g1_type::value_type p2){

    using main_curve_type = CurveType;
    using chained_curve_type = typename CurveType::pairing::chained_curve_type;
    using scalar_field_type = typename CurveType::scalar_field_type;

    // Create blueprint

    blueprint<scalar_field_type> bp;

    blueprint_variable<scalar_field_type> a;
    blueprint_variable<scalar_field_type> d;

    a.allocate(bp);
    d.allocate(bp);

    bp.val(a) = chained_curve_type::a;
    bp.val(d) = chained_curve_type::d;

    components::element_g1<main_curve_type> P1(bp, p1);
    components::element_g1<main_curve_type> P2(bp, p2);
    components::element_g1<main_curve_type> P1pP2(bp);

    bp.set_input_sizes(2);

    std::cout << "element_g1 size: " << P1.num_variables() << std::endl;

    std::cout << "blueprint size: " << bp.num_variables() << std::endl;

    print_fp_curve_group_element(p1);

    std::cout << "P1:" << std::endl;
    print_field_element(bp.lc_val(P1.X));
    print_field_element(bp.lc_val(P1.Y));

    std::cout << "P2:" << std::endl;
    print_field_element(bp.lc_val(P2.X));
    print_field_element(bp.lc_val(P2.Y));

    components::element_g1_add<main_curve_type> el_add(bp, a, d, P1, P2, P1pP2);

    el_add.generate_r1cs_constraints();
    el_add.generate_r1cs_witness();

    std::cout << "P1pP2:" << std::endl;
    print_field_element(bp.lc_val(P1pP2.X));
    print_field_element(bp.lc_val(P1pP2.Y));

    std::cout << "P1 + P2:" << std::endl;
    print_fp_curve_group_element(p1 + p2);

    std::cout << "blueprint size: " << bp.num_variables() << std::endl;

    return bp;
}

int main(){

    using main_curve_type = curves::bls12<381>;
    using chained_curve_type = typename main_curve_type::pairing::chained_curve_type;
    using scalar_field_type = typename main_curve_type::scalar_field_type;

    typename chained_curve_type::g1_type::value_type p1 = 
        typename chained_curve_type::g1_type::value_type(
        random_element<typename chained_curve_type::g1_type::underlying_field_type>(),
        random_element<typename chained_curve_type::g1_type::underlying_field_type>());
    typename chained_curve_type::g1_type::value_type p2 = 
        typename chained_curve_type::g1_type::value_type(
        random_element<typename chained_curve_type::g1_type::underlying_field_type>(),
        random_element<typename chained_curve_type::g1_type::underlying_field_type>());

    blueprint<scalar_field_type> bp = addition_example<main_curve_type>(p1, p2);

    assert(bp.is_satisfied());

    verify_component<main_curve_type>(bp);

    return 0;
}
