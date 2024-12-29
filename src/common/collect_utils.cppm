module;

import stl;
#include <atomic>

export module collect_utils;

export namespace fast::util {

/**
 * @note
 * map<string, int>::key_type  v1; //此时key_type的v1类型是string
 * map<string, int>::mapped_type  v2; //此时mapped_type的v2类型是int
 * map<string, int>::value_type  v3; //此时value_type的v3类型是pair<const
 * string, int>，注意pair键值对中第一个元素的类型有const(不可修改).
 */
class MapUtil {
public:
  /**
   *@brief
   *给定一个映射和一个键，返回对应于映射中的键的值，如果键不存在于映射中，则返回给定的默认值。
   *@code <pre>
   *
   *  MapUtil map_util;
   *
   *  std::map<std::string, std::string> map{
   *	    {"this", "100"},
   *  	{"can", "100"},
   *      {"be", "100"},
   *      {"const", "100"},
   *	};
   *
   *	auto default_value = map_util.GetDefault(map, "undefined");
   *
   *	LOG_INFO << typeid(default_value).name();
   *
   *
   * </pre>
   */
  template <typename Map, typename Key>
  static typename Map::mapped_type GetDefault(const Map &map, const Key &key) {
    auto pos = map.find(key);
    return (pos != map.end()) ? (pos->second) : (typename Map::mapped_type{});
  }

  /**
   *@brief
   *给定一个映射和一个键，返回对映射中与键对应的值的引用，如果映射中不存在该键，则返回给定的默认引用。
   *@code <pre>
   *
   *  MapUtil map_util;
   *
   *  std::map<std::string, std::string> map{
   *	  {"this", "100"},
   *  	  {"can", "100"},
   *      {"be", "100"},
   *      {"const", "100"},
   *	};
   *
   *  std::string mapped_type_value = "abcd";
   *  auto mapped_type_default_value = map_util.GetRefDefault(map, "undefined",
   *mapped_type_value);
   *
   *  LOG_INFO << typeid(mapped_type_default_value).name();
   *
   *  LOG_INFO << mapped_type_default_value;
   *
   *  if (mapped_type_default_value == mapped_type_value) {
   *	 LOG_INFO << &mapped_type_default_value << "value is equal,but memory
   *address not equal" << &mapped_type_value;
   *  }
   * </pre>
   */
  template <class Map, typename Key = typename Map::key_type>
  static const typename Map::mapped_type &
  GetRefDefault(const Map &map, const Key &key,
                const typename Map::mapped_type &dflt) {
    auto pos = map.find(key);
    return (pos != map.end() ? pos->second : dflt);
  }

  /**
   * 传递临时默认值会在返回时返回一个悬空引用，生命周期延长被间接破坏了。
   * 调用者必须确保默认值比 GetRefDefault() 返回的引用生命周期长。
   */
  template <class Map, typename Key = typename Map::key_type>
  static const typename Map::mapped_type &
  GetRefDefault(const Map &map, const Key &key,
                typename Map::mapped_type &&dflt) = delete;

  template <class Map, typename Key = typename Map::key_type>
  static const typename Map::mapped_type &
  GetRefDefault(const Map &map, const Key &key,
                const typename Map::mapped_type &&dflt) = delete;

  /**
   *@brief
   *给定一个映射和一个键，返回一个指向对应于映射中键的值的指针，如果键不存在于映射中，则返回
   *nullptr。
   *@code <pre>
   *
   *  MapUtil map_util;
   *
   *  std::map<std::string, std::string> map{
   *	    {"this", "100"},
   *  	{"can", "100"},
   *      {"be", "100"},
   *      {"const", "100"},
   *	};
   *
   *  // 默认 auto 带有const
   *  auto *const_ptr= map_util.GetConstPtr(map, "this");
   *
   * </pre>
   */
  template <class Map, typename Key = typename Map::key_type>
  static const auto *GetConstPtr(const Map &map, const Key &key) {
    auto pos = map.find(key);
    return (pos != map.end() ? &pos->second : nullptr);
  }

  /**
   *@brief 非const 获取 ptr
   *@code <pre>
   *
   *  MapUtil map_util;
   *
   *  std::map<std::string, std::string> map{
   *	  {"this", "100"},
   *  	  {"can", "100"},
   *      {"be", "100"},
   *      {"const", "100"},
   *	};
   *
   *  auto *ptr = map_util.GetPtr(map, "this");
   *
   *  *ptr = "200";
   *
   *  LOG_INFO << map["this"];
   * </pre>
   */
  template <class Map, typename Key = typename Map::key_type>
  static auto *GetPtr(Map &map, const Key &key) {
    auto pos = map.find(key);
    return (pos != map.end() ? &pos->second : nullptr);
  }

  template <class Map, typename Key = typename Map::key_type>
  static void printfMap(Map &map, const Key &key) {
    for (const auto &item : map) {

      // LOG_INFO << item.first << "   " << item.second;
    }
  }

}; // MapUtil

template <typename K, typename V> struct Node {
  K key;
  V value;
  Node *next;

  Node(const K &k, const V &v) : key(k), value(v), next(nullptr) {}
};

template <typename K, typename V> struct Bucket {
  std::atomic<Node<K, V> *> head;
  std::atomic<Node<K, V> *> tail;
  std::mutex bucket_mutex; // Mutex for fine-grained locking

  Bucket() : head(nullptr), tail(nullptr) {}
};

template <typename K, typename V, typename Hash = std::hash<K>>
class LockFreeMap {
private:
  static constexpr size_t DEFAULT_BUCKETS = 10;
  static constexpr size_t GROW_THRESHOLD  = 4; // Threshold to trigger resizing

  std::vector<Bucket<K, V>> table;
  Hash hasher;
  std::atomic<size_t> num_elements;
  std::mutex resize_mutex;

  size_t get_bucket_index(const K &key) const {
    return hasher(key) % table.size();
  }

  void resize() {
    std::lock_guard<std::mutex> resize_lock(resize_mutex);
    std::vector<Bucket<K, V>> new_table(table.size() * 2);

    for (size_t i = 0; i < table.size(); ++i) {
      Node<K, V> *current = table[i].head.load();
      while (current != nullptr) {
        size_t index     = hasher(current->key) % new_table.size();
        Node<K, V> *next = current->next;
        if (new_table[index].tail != nullptr) {
          new_table[index].tail.load()->next = current;
        } else {
          new_table[index].head.store(current);
        }
        new_table[index].tail.store(current);
        current->next = nullptr;
        current       = next;
      }
    }
    table = std::move(new_table);
  }

public:
  LockFreeMap(size_t num_buckets = DEFAULT_BUCKETS)
      : table(num_buckets), num_elements(0) {}

  ~LockFreeMap() { clear(); }

  void insert(const K &key, const V &value) {
    if (num_elements.load() >= GROW_THRESHOLD * table.size()) {
      resize();
    }

    size_t index         = get_bucket_index(key);
    Node<K, V> *new_node = new Node<K, V>(key, value);

    Bucket<K, V> &bucket = table[index];
    std::lock_guard<std::mutex> lock(bucket.bucket_mutex);

    Node<K, V> *old_tail = bucket.tail.exchange(new_node);
    if (old_tail != nullptr) {
      old_tail->next = new_node;
    } else {
      bucket.head.store(new_node);
    }
    ++num_elements;
  }

  bool find(const K &key, V &value) {
    size_t index         = get_bucket_index(key);

    Bucket<K, V> &bucket = table[index];
    Node<K, V> *current  = bucket.head.load();

    while (current != nullptr) {
      if (current->key == key) {
        value = current->value;
        return true;
      }
      current = current->next;
    }
    return false;
  }

  bool remove(const K &key) {
    size_t index         = get_bucket_index(key);
    Bucket<K, V> &bucket = table[index];
    std::lock_guard<std::mutex> lock(bucket.bucket_mutex);

    Node<K, V> *current = bucket.head.load();
    Node<K, V> *prev    = nullptr;
    while (current != nullptr) {
      if (current->key == key) {
        if (prev == nullptr) {
          bucket.head.store(current->next);
        } else {
          prev->next = current->next;
        }
        if (current == bucket.tail) {
          bucket.tail.store(prev);
        }
        delete current;
        --num_elements;
        return true;
      }
      prev    = current;
      current = current->next;
    }
    return false;
  }

  void clear() {
    for (auto &bucket : table) {
      Node<K, V> *current = bucket.head.load();
      while (current != nullptr) {
        Node<K, V> *next = current->next;
        delete current;
        current = next;
      }
      bucket.head.store(nullptr);
      bucket.tail.store(nullptr);
    }
    num_elements.store(0);
  }

  size_t size() const { return num_elements.load(); }

  void traverse(std::function<void(const K &, const V &)> func) {
    for (auto &bucket : table) {
      Node<K, V> *current = bucket.head.load();
      while (current != nullptr) {
        func(current->key, current->value);
        current = current->next;
      }
    }
  }
};

template <typename T> class LockFreeList {
private:
  struct Node {
    T data;
    std::atomic<Node *> next;

    Node(const T &val) : data(val), next(nullptr) {}
  };

  std::atomic<Node *> head;
  std::atomic<size_t> listSize;

public:
  LockFreeList() : head(nullptr), listSize(0) {}

  ~LockFreeList() {
    Node *curr = head.load();
    while (curr) {
      Node *next = curr->next.load();
      delete curr;
      curr = next;
    }
  }

  void insert(const T &val) {
    Node *newNode  = new Node(val);
    Node *expected = head.load(); // 初始化期望的值为当前的头节点

    do {
      newNode->next = expected; // 设置新节点的 next 指针为当前期望的头节点
    } while (
        !head.compare_exchange_weak(expected, newNode)); // 尝试原子地更新头节点

    listSize.fetch_add(1);
  }

  bool remove(const T &val) {
    Node *curr = head.load();
    Node *prev = nullptr;

    while (curr != nullptr) {
      if (curr->data == val) {
        if (prev)
          prev->next = curr->next.load();
        else
          head = curr->next.load();
        delete curr;
        listSize.fetch_sub(1);
        return true;
      }
      prev = curr;
      curr = curr->next.load();
    }
    return false;
    }

    size_t size() const { return listSize.load(); }

    void display()
    {
        Node *curr = head.load();
        while (curr) {
            std::cout << curr->data << " ";
            curr = curr->next.load();
        }
        std::cout << std::endl;
    }
};
}