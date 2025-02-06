
module;

//import stl;
#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

export module collect_expiration_utils;

/**
 *   // 默认1分钟 开始检测一次过期
    template<typename K, typename V, typename Hash>
    const std::chrono::seconds LockFreeMapExprise<K, V,
Hash>::EXPIRATION_CHECK_INTERVAL = std::chrono::seconds(1);

   int main() {
    LockFreeMapExprise<int, std::string> lfmap;

    // Insert some elements
    for (int i = 0; i < 1000000; ++i) {
        lfmap.insert(i, "Value" + std::to_string(i));
    }

    // Print map size
    std::cout << "Map size: " << lfmap.size() << std::endl;

    // Wait for a while to allow expiration task to clear expired keys
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Print map size after expiration
    std::cout << "Map size after expiration: " << lfmap.size() << std::endl;

    return 0;
}
 */
export namespace fast::util {

template <typename K, typename V> struct Node {
  K key;
  V value;
  std::chrono::time_point<std::chrono::system_clock> timestamp;
  Node *next;

  Node(const K &k, const V &v)
      : key(k), value(v), timestamp(std::chrono::system_clock::now()),
        next(nullptr) {}
};

template <typename K, typename V> struct Bucket {
  std::atomic<Node<K, V> *> head;
  std::mutex bucket_mutex; // Mutex for fine-grained locking

  Bucket() : head(nullptr) {}
};

template <typename K, typename V, typename Hash = std::hash<K>>
class LockFreeMapExprise {
private:
  static const size_t DEFAULT_BUCKETS = 10;
  static const size_t GROW_THRESHOLD  = 4; // Threshold to trigger resizing
  static const std::chrono::seconds EXPIRATION_CHECK_INTERVAL;

  std::vector<Bucket<K, V>> table;
  Hash hasher;
  std::atomic<size_t> num_elements;
  std::thread expiration_thread;
  std::atomic<bool> expiration_thread_running;

  size_t get_bucket_index(const K &key) const {
    return hasher(key) % table.size();
  }

  void resize() {
    size_t new_size = table.size() * 2;
    std::vector<Bucket<K, V>> new_table(new_size);
    for (size_t i = 0; i < table.size(); ++i) {
      Node<K, V> *current = table[i].head.load();
      while (current != nullptr) {
        size_t new_index = hasher(current->key) % new_size;
        Node<K, V> *next = current->next;
        current->next    = new_table[new_index].head.load();
        new_table[new_index].head.store(current);
        current = next;
      }
    }
    table.swap(new_table);
  }

  void expiration_task() {
    while (expiration_thread_running) {
      std::this_thread::sleep_for(EXPIRATION_CHECK_INTERVAL);
      auto now = std::chrono::system_clock::now();
      for (auto &bucket : table) {
        std::lock_guard<std::mutex> lock(bucket.bucket_mutex);
        Node<K, V> **current = &bucket.head;
        while (*current != nullptr) {
          if ((now - (*current)->timestamp) > EXPIRATION_CHECK_INTERVAL) {
            Node<K, V> *expired_node = *current;
            *current                 = expired_node->next;
            delete expired_node;
            --num_elements;
          } else {
            current = &((*current)->next);
          }
        }
      }
    }
  }

public:
  LockFreeMapExprise(size_t num_buckets = DEFAULT_BUCKETS)
      : table(num_buckets), num_elements(0), expiration_thread_running(true) {
    expiration_thread = std::thread(&LockFreeMapExprise::expiration_task, this);
  }

  ~LockFreeMapExprise() {
    expiration_thread_running = false;
    expiration_thread.join();
    clear();
  }

  void insert(const K &key, const V &value) {
    size_t index         = get_bucket_index(key);
    Node<K, V> *new_node = new Node<K, V>(key, value);

    Bucket<K, V> &bucket = table[index];
    std::lock_guard<std::mutex> lock(bucket.bucket_mutex);

    new_node->next = bucket.head.load();
    bucket.head.store(new_node);
    ++num_elements;

    if (num_elements.load() >= GROW_THRESHOLD * table.size()) {
      resize();
    }
  }

  bool find(const K &key, V &value) {
    size_t index         = get_bucket_index(key);

    Bucket<K, V> &bucket = table[index];
    std::lock_guard<std::mutex> lock(bucket.bucket_mutex);

    Node<K, V> *current = bucket.head.load();
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

// 默认1分钟 开始检测一次过期
template <typename K, typename V, typename Hash>
const std::chrono::seconds
    LockFreeMapExprise<K, V, Hash>::EXPIRATION_CHECK_INTERVAL =
        std::chrono::seconds(1);

}
