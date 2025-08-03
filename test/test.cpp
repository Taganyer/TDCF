//
// Created by taganyer on 25-8-3.
//

#include <test/test.hpp>

#include <test/manager/TestManager.hpp>
#include <test/frame1/ComponentCreator1.hpp>

using namespace test;

void test::example() {
    TestManager manager(ComponentCreator1::get());

    ClusterInfoPtr star = ClusterInfo::get(ClusterInfo::Star, 0, 1, 1, 1, 1, 1);

    ClusterInfoPtr ring = ClusterInfo::get(ClusterInfo::Ring, 20, 1, 1, 1, 1, 1);

    ClusterInfoPtr dbt = ClusterInfo::get(ClusterInfo::DBT, 20, 1, 1, 1, 1, 1);

    star->add_sub_cluster(std::move(ring));
    star->add_sub_cluster(std::move(dbt));

    manager.run(*star);

}
