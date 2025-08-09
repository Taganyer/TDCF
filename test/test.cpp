//
// Created by taganyer on 25-8-3.
//

#include <test/test.hpp>

#include <test/manager/TestManager.hpp>
#include <test/frame1/ComponentCreator1.hpp>

#include "frame1/Identity1.hpp"

using namespace test;

enum class RootType {
    star,
    ring,
    dbt
};

static void run(ClusterInfoPtr& star, ClusterInfoPtr& ring, ClusterInfoPtr& dbt, RootType root) {
    TestManager manager(ComponentCreator1::get());
    switch (root) {
        case RootType::star:
            star->add_sub_cluster(std::move(ring));
            star->add_sub_cluster(std::move(dbt));
            manager.run(*star);
            break;
        case RootType::ring:
            ring->add_sub_cluster(std::move(star));
            ring->add_sub_cluster(std::move(dbt));
            manager.run(*ring);
            break;
        case RootType::dbt:
            dbt->add_sub_cluster(std::move(star));
            dbt->add_sub_cluster(std::move(ring));
            manager.run(*dbt);
            break;
    }
}

void test::example() {
    ClusterInfoPtr star = ClusterInfo::get(ClusterInfo::Star, 10, 1, 1, 1, 1, 1);

    ClusterInfoPtr ring = ClusterInfo::get(ClusterInfo::Ring, 10, 1, 1, 1, 1, 1);

    ClusterInfoPtr dbt = ClusterInfo::get(ClusterInfo::DBT, 10, 1, 1, 1, 1, 1);

    run(star, ring, dbt, RootType::dbt);

}

void test::tmp() {
    CommShare cs;
    Communicator1 c1(0, cs);
    tdcf::SerializablePtr ptr = std::make_shared<tdcf::Data>();
    tdcf::IdentityPtr id = std::make_shared<Identity1>(1);
    c1.send_message(id, tdcf::Message(tdcf::MetaData()), ptr);
}
