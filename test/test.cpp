//
// Created by taganyer on 25-8-3.
//

#include <test/test.hpp>

#include <test/manager/TestManager.hpp>
#include <test/frame1/ComponentCreator1.hpp>

using namespace test;

void test::example() {
    TestManager manager(ComponentCreator1::get());

    ClusterInfoPtr star = ClusterInfo::get(ClusterInfo::Star, 1, 1, 1, 1, 1, 1);

    ClusterInfoPtr ring = ClusterInfo::get(ClusterInfo::Ring, 1, 1, 1, 1, 1, 1);

    ClusterInfoPtr dbt = ClusterInfo::get(ClusterInfo::DBT, 1, 1, 1, 1, 1, 1);

    // star->add_sub_cluster(std::move(ring));
    // star->add_sub_cluster(std::move(dbt));

    // ring->add_sub_cluster(std::move(star));
    // ring->add_sub_cluster(std::move(dbt));

    dbt->add_sub_cluster(std::move(star));
    dbt->add_sub_cluster(std::move(ring));

    manager.run(*dbt);

}
