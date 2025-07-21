//
// Created by taganyer on 25-7-16.
//
#pragma once

#include <iostream>
#include <stack>
#include <tdcf/base/DBT/DBT.hpp>

namespace test {

    inline void print(const tdcf::dbt::DBTNodeInfo& info) {
        std::cerr << info.t1_parent << ' '
            << info.t1_left << ' '
            << info.t1_right << ' '
            << info.t1_color << " --- "
            << info.t2_parent << ' '
            << info.t2_left << ' '
            << info.t2_right << ' '
            << info.t2_color << std::endl;
    }

    inline void DBT_correctness_test() {
        for (uint32_t i = 2; i < 100; ++i) {
            std::vector<bool> vis1(i, false), vis2(i, false);
            auto [root1, root2, array] = tdcf::dbt::creat_dbt(i);
            for (uint32_t j = 0; j < i; ++j) {
                std::cerr << j << ": ";
                print(array[j]);
            }
            std::cerr << std::endl;
            assert(array[root1].t1_parent == -1 && array[root2].t2_parent == -1);
            std::stack<uint32_t> st;
            st.push(root1);
            while (!st.empty()) {
                uint32_t parent = st.top();
                st.pop();
                if (parent == -1) continue;
                assert(!vis1[parent]);
                vis1[parent] = true;
                st.push(array[parent].t1_left);
                st.push(array[parent].t1_right);
            }
            for (uint32_t j = 0; j < i; ++j) {
                assert(vis1[j]);
            }

            st.push(root2);
            while (!st.empty()) {
                uint32_t parent = st.top();
                st.pop();
                if (parent == -1) continue;
                assert(!vis2[parent]);
                vis2[parent] = true;
                st.push(array[parent].t2_left);
                st.push(array[parent].t2_right);
            }
            for (uint32_t j = 0; j < i; ++j) {
                assert(vis2[j]);
            }

            for (auto& node : array) {
                assert(node.t1_color < 2 && node.t2_color < 2);

                assert(node.t1_color != node.t2_color);

                if (node.t1_left != -1 && node.t1_right != -1) {
                    assert(array[node.t1_left].t1_color != array[node.t1_right].t1_color);
                }

                if (node.t2_left != -1 && node.t2_right != -1) {
                    assert(array[node.t2_left].t2_color != array[node.t2_right].t2_color);
                }
            }
        }
    };

}
