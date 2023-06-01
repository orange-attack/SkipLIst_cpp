#include <iostream>
#include "skiplist.h"
#define FILE_PATH "./store/dumpFile"

int main() {

    // 键值中的key用int型，如果用其他类型，需要自定义比较函数
    // 而且如果修改key的类型，同时需要修改skipList.load_file函数
    // 定义最高索引层为6的跳表
    SkipList<int, std::string> skipList(6);
	skipList.insert_element(1, "在小小的花园里，"); 
	skipList.insert_element(3, "挖呀挖呀挖,"); 
	skipList.insert_element(7, "种小小的种子,"); 
	skipList.insert_element(8, "开小小的花,"); 
	skipList.insert_element(9, "在大大的花园里，挖呀挖呀挖,"); 
	skipList.insert_element(18, "种大大的种子,"); 
	skipList.insert_element(19, "开大大的花。"); 

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.dump_file();

    // skipList.load_file();

    skipList.search_element(9);
    skipList.search_element(18);


    skipList.display_list();

    skipList.delete_element(3);
    skipList.delete_element(7);

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.display_list();
}
