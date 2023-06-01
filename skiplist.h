#include <iostream> 
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>

#define STORE_FILE "store/dumpFile"

std::mutex mtx;     // 临界区互斥锁
std::string delimiter = ":"; //分隔符

// 跳表结构中的节点类
template<typename K, typename V> 
class Node {

public:
    
    Node() {} 

    Node(K k, V v, int); 

    ~Node();

    K get_key() const;

    V get_value() const;

    void set_value(V);
    
    // 用于保存 指向 不同层的下一个节点的指针 的线性数组
    Node<K, V> **forward;

    // 所在层级
    int node_level; 

private:
    K key;
    V value;
};

// 节点的有参构造（k, v, 所在层级-随机生成）
template<typename K, typename V> 
Node<K, V>::Node(const K k, const V v, int level) {
    this->key = k;
    this->value = v;
    this->node_level = level; 

    // level + 1
    // 索引层 0 ~ level 
    this->forward = new Node<K, V>*[level+1];
    
	// 初始化：用 0（NULL) 填充数组
    // void *memset( void *buffer, int ch, size_t count );
    // 拷贝ch 到buffer从头开始的count个字符里，并返回buffer的指针
    memset(this->forward, 0, sizeof(Node<K, V>*)*(level+1)); //将数组的分量全部设置为 0
};

template<typename K, typename V> 
Node<K, V>::~Node() {
    delete []forward;
};

template<typename K, typename V> 
K Node<K, V>::get_key() const {
    return key;
};

template<typename K, typename V> 
V Node<K, V>::get_value() const {
    return value;
};
template<typename K, typename V> 
void Node<K, V>::set_value(V value) {
    this->value=value;
};

// 跳表类
template <typename K, typename V> 
class SkipList {

public: 
    SkipList(int);
    ~SkipList();
    int get_random_level();
    Node<K, V>* create_node(K, V, int);
    int insert_element(K, V);
    void display_list();
    bool search_element(K);
    void delete_element(K);
    void dump_file();
    void load_file();
    int size();

private:
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
    bool is_valid_string(const std::string& str);

private:    
    // 跳表索引的最高层 
    int _max_level;

    // 当前索引层数 
    int _skip_list_level;
 
    // 头结点
    Node<K, V> *_header;

    // 文件操作
    std::ofstream _file_writer;
    std::ifstream _file_reader;

    // 跳表当前的元素个数
    int _element_count;
};
 
// 创建新的节点
template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K k, const V v, int level) {
    Node<K, V> *n = new Node<K, V>(k, v, level);
    return n;
}

// 在跳表中插入元素 
// return 1 代表元素已存在  
// return 0 代表插入成功
/* 
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+

*/
template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value) {
    
    mtx.lock();
    Node<K, V> *current = this->_header;

    // 创建 update 数组 并初始化 
    // update 数组 存放 当前层最后一个key小于我们要插入节点的key的节点
    Node<K, V> *update[_max_level+1];
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));  

    // 从最高层开始遍历 
    for(int i = _skip_list_level; i >= 0; i--) {
        while(current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i]; 
        }
        // 保存
        update[i] = current;
        // 切换到下一层
    }

    // 将current指向第0层第一个不小于要插入节点key的节点
    current = current->forward[0];

    // 节点已存在
    if (current != NULL && current->get_key() == key) {
        std::cout << "key: " << key << ", exists" << std::endl;
        mtx.unlock();
        return 1;
    }

    // 如果current为空，则 we have reached to end of the level 
    // 如果 current's key 不等于要插入的节点的key则 该节点插在 update[0] 和 current node 之间 
    if (current == NULL || current->get_key() != key ) {
        
        // 为当前要插入的节点生成一个随机层数
        int random_level = get_random_level();

        // 如果随机层数比当前的层数大, 初始化 update 值并指向header
        if (random_level > _skip_list_level) {
            for (int i = _skip_list_level+1; i < random_level+1; i++) {
                update[i] = _header;
            }
            // 更新层数
            _skip_list_level = random_level;
        }

        // 创建要插入的节点 
        Node<K, V>* inserted_node = create_node(key, value, random_level);
        
        // 插入节点 
        for (int i = 0; i <= random_level; i++) {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
        _element_count ++;
    }
    mtx.unlock();
    return 0;
}

// 展示跳表
template<typename K, typename V> 
void SkipList<K, V>::display_list() {

    std::cout << "\n*****Skip List*****"<<"\n"; 
    for (int i = 0; i <= _skip_list_level; i++) {
        Node<K, V> *node = this->_header->forward[i]; 
        std::cout << "Level " << i << ": ";
        while (node != NULL) {
            std::cout << node->get_key() << ":" << node->get_value() << ";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}


// 将内存中的数据存储在文件中
template<typename K, typename V> 
void SkipList<K, V>::dump_file() {

    std::cout << "dump_file-----------------" << std::endl;
    _file_writer.open(STORE_FILE);
    Node<K, V> *node = this->_header->forward[0]; 

    while (node != NULL) {
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";
        node = node->forward[0];
    }

    _file_writer.flush();
    _file_writer.close();
    return ;
}

// 从文件中加载数据
template<typename K, typename V> 
void SkipList<K, V>::load_file() {

    _file_reader.open(STORE_FILE);
    std::cout << "load_file-----------------" << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    while (getline(_file_reader, line)) {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty()) {
            continue;
        }
        insert_element(*key, *value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    _file_reader.close();
}

// 获取元素的数量 
template<typename K, typename V> 
int SkipList<K, V>::size() { 
    return _element_count;
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {

    if(!is_valid_string(str)) {
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter)+1, str.length());
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {

    if (str.empty()) {
        return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}

// 删除元素 
template<typename K, typename V> 
void SkipList<K, V>::delete_element(K key) {

    mtx.lock();
    Node<K, V> *current = this->_header; 
    Node<K, V> *update[_max_level+1];
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));

    // 从最高层开始遍历
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] !=NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];
    if (current != NULL && current->get_key() == key) {
       
        // 从第0层开始，删除每一层中该节点
        for (int i = 0; i <= _skip_list_level; i++) {

            // 如果在 i 层, 下一个节点不是目标节点, 打破循环
            if (update[i]->forward[i] != current) 
                break;

            update[i]->forward[i] = current->forward[i];
        }

        // 移除索引层中没有元素的层
        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
            _skip_list_level --; 
        }

        std::cout << "Successfully deleted key "<< key << std::endl;
        _element_count --;
    }
    mtx.unlock();
    return;
}

// 在跳表中查找元素 
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
template<typename K, typename V> 
bool SkipList<K, V>::search_element(K key) {

    std::cout << "search_element-----------------" << std::endl;
    Node<K, V> *current = _header;

    // 从跳表的最高层开始遍历
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
    }

    // 在第 0 索引层 current 为查找的节点
    current = current->forward[0];

    // 判断键值是否是我们查找的
    if (current and current->get_key() == key) {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

// 跳表的构造函数
template<typename K, typename V> 
SkipList<K, V>::SkipList(int max_level) {

    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;

    // 创建头节点，并初始化k v 为null
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
};

// 跳表的析构函数
template<typename K, typename V> 
SkipList<K, V>::~SkipList() {

    if (_file_writer.is_open()) {
        _file_writer.close();
    }
    if (_file_reader.is_open()) {
        _file_reader.close();
    }
    delete _header;
}

// 生成随机层数
template<typename K, typename V>
int SkipList<K, V>::get_random_level(){

    int k = 1;
    // 没有随机数种子，则每次生成的随机数一样
    while (rand() % 2) {
        k++;
    }
    
    // 最高层限制
    k = (k < _max_level) ? k : _max_level;
    return k;
};
// vim: et tw=100 ts=4 sw=4 cc=120
