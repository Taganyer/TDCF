//
// Created by taganyer on 25-7-16.
//
#pragma once

#include <cassert>
#include <cstdint>
#include <vector>

namespace tdcf::dbt {

    inline uint32_t get_root(uint32_t begin, uint32_t size) {
        assert(size > 0);
        uint32_t step = 0;
        for (uint32_t total = 2; total <= size; total <<= 1) { ++step; }
        return begin + (1UL << step) - 1;
    }


    struct DBTNodeInfo {
        uint32_t t1_parent = -1, t1_left = -1, t1_right = -1, t1_color = -1;
        uint32_t t2_parent = -1, t2_left = -1, t2_right = -1, t2_color = -1;
    };

    using DBTArray = std::vector<DBTNodeInfo>;

    struct DBTInfo {
        uint32_t root1 = -1, root2 = -1;
        DBTArray array;
    };


    namespace details {

        inline uint32_t __build_tree1(DBTArray& array, uint32_t begin, uint32_t size) {
            if (size == 0) return -1;
            uint32_t root = get_root(begin, size);
            uint32_t left_size = root - begin;
            uint32_t left = __build_tree1(array, begin, left_size);
            uint32_t right = __build_tree1(array, root + 1, size - left_size - 1);
            array[root].t1_left = left;
            array[root].t1_right = right;
            if (left != -1) {
                array[left].t1_parent = root;
            }
            if (right != -1) {
                array[right].t1_parent = root;
            }
            return root;
        }

        inline uint32_t __build_tree2_even(DBTArray& array) {
            uint32_t size = array.size() - 1;
            for (uint32_t i = 0; i <= size; ++i) {
                uint32_t mirror = size - i;
                uint32_t left_mirror = array[mirror].t1_left != -1 ? size - array[mirror].t1_left : -1;
                uint32_t right_mirror = array[mirror].t1_right != -1 ? size - array[mirror].t1_right : -1;
                array[i].t2_left = left_mirror;
                array[i].t2_right = right_mirror;
                if (left_mirror != -1) {
                    array[left_mirror].t2_parent = i;
                }
                if (right_mirror != -1) {
                    array[right_mirror].t2_parent = i;
                }
            }
            return size - get_root(0, size + 1);
        }

        inline uint32_t __build_tree2(DBTArray& array) {
            uint32_t size = array.size();
            for (uint32_t i = 0; i < size; ++i) {
                uint32_t shift = (i + size - 1) % size;
                array[i].t2_left = array[shift].t1_left != -1 ? (array[shift].t1_left + 1) % size : -1;
                array[i].t2_right = array[shift].t1_right != -1 ? (array[shift].t1_right + 1) % size : -1;
                if (array[i].t2_left != -1) {
                    array[array[i].t2_left].t2_parent = i;
                }
                if (array[i].t2_right != -1) {
                    array[array[i].t2_right].t2_parent = i;
                }
            }
            return (get_root(0, size) + 1) % size;
        }

        inline uint32_t __dyeing_tree1_even(DBTArray& array, uint32_t index) {
            if (array[index].t1_color != -1) return array[index].t1_color;

            uint32_t parent_color = __dyeing_tree1_even(array, array[index].t1_parent);
            return array[index].t1_color =
                parent_color ^
                (array[index].t1_parent > index);
        }

        inline void __dyeing_tree2(DBTArray& array) {
            for (auto& info : array) {
                assert(info.t1_color == 0 || info.t1_color == 1);
                info.t2_color = 1 - info.t1_color;
            }
        }

        inline bool check(const DBTArray& array) {
            for (auto& node : array) {
                assert(node.t1_color < 2 && node.t2_color < 2);

                assert(node.t1_color != node.t2_color);

                if (node.t1_left != -1 && node.t1_right != -1) {
                    if (array[node.t1_left].t1_color == array[node.t1_right].t1_color)
                        return false;
                }

                if (node.t2_left != -1 && node.t2_right != -1) {
                    if (array[node.t2_left].t2_color == array[node.t2_right].t2_color)
                        return false;
                }
            }
            return true;
        }

        inline bool __dyeing(uint32_t index, DBTArray& array) {
            if (index == array.size()) {
                __dyeing_tree2(array);
                return check(array);
            }
            auto& node = array[index];
            if (node.t1_left != -1 && node.t1_right != -1) {
                array[node.t1_left].t1_color = 0;
                array[node.t1_right].t1_color = 1;
                if (__dyeing(index + 1, array)) return true;
                array[node.t1_left].t1_color = 1;
                array[node.t1_right].t1_color = 0;
            } else if (node.t1_left != -1) {
                array[node.t1_left].t1_color = 0;
                if (__dyeing(index + 1, array)) return true;
                array[node.t1_left].t1_color = 1;
            } else if (node.t1_right != -1) {
                array[node.t1_right].t1_color = 0;
                if (__dyeing(index + 1, array)) return true;
                array[node.t1_right].t1_color = 1;
            }
            if (__dyeing(index + 1, array)) return true;
            return false;
        }

    }

    inline DBTInfo creat_dbt(uint32_t size) {
        DBTArray array(size);
        uint32_t root1 = details::__build_tree1(array, 0, size);
        uint32_t root2 = details::__build_tree2(array);
        array[root1].t1_color = 1;
        bool success = details::__dyeing(0, array);
        assert(success);
        return { root1, root2, std::move(array) };
    }

}
