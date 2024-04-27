#include "forward_list.cpp"

int main(void) {
    beststd::forward_list<int> l;
    std::cout << "Is empty: " << l.empty() << "\n";
    std::cout << "Size: " << l.size() << "\n";
    l.push_front(3); 
    l.push_front(2);
    l.push_front(1);
    auto iter = l.begin();
    l.insert_after(10, iter);
    l.erase_after(++iter);
    l.insert_after(15, iter);
    for (auto it = l.begin(); it != l.end(); it++) {
        std::cout << *it << " ";
    }
    std::cout << "\nIs empty: " << l.empty() << "\n";
    std::cout << "Size: " << l.size() << "\n";
    //Нахождение элемента
    auto elem = l.find(12);
    if (elem != l.end()) {
        std::cout << "Element finded: " << *elem << "\n";
    } else {
        std::cout << "Cant find element\n";
    }
    l.clear();
    std::cout << "Is empty: " << l.empty() << "\n";
    std::cout << "Size: " << l.size() << "\n";
    return 0;
}