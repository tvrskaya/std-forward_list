#include <iostream>
#include <condition_variable>

namespace beststd { // не засоряем глобальное пространство имен
    
    template<typename T>
    class forward_list {
    private:
        struct BaseNode {
            BaseNode* next;
        };

        struct Node : BaseNode {
            T value;
        };

        size_t sz;
        //fake node. Является следующим элементом после последнего и указывает на первый
        BaseNode end_; 

        //После реализации операций над листом в один момент возникает необходимость
        //обращаться к самим элементам листа, поэтому становится понятная необходимость
        //некоторой абстракции над содержимым контейнера - итератора
        template <bool IsConst> //Реализуем итератор и конст_итератор без копирования кода класса
        class base_iterator {
        friend forward_list;
        public:
            using pointer_type = std::conditional_t<IsConst, const T*, T*>;
            using reference_type = std::conditional_t<IsConst, const T&, T&>;
            using value_type = T;

        private:
            std::conditional_t<IsConst, const BaseNode*, BaseNode*> ptr;
            base_iterator(BaseNode* ptr) : ptr(ptr) {}
             
        public:
            base_iterator(const base_iterator&) = default;
            base_iterator& operator=(const base_iterator&) = default;
            constexpr bool operator!=(const base_iterator& other) { 
                return !(*this == other);
            }
            constexpr bool operator==(const base_iterator& other) {
                return this->ptr == other.ptr;
            }
            reference_type operator*() { return static_cast<Node*>(ptr)->value; }
            pointer_type operator->() { return &static_cast<Node*>(ptr)->value; }

            base_iterator& operator++() { 
                ptr = ptr->next; 
                return *this; 
            }

            base_iterator operator++(int) {
                base_iterator copy = *this;
                ptr = ptr->next;
                return copy;
            }
        };

    public:
        //Таким образом, получаем const_iterator и iterator без повторения кода
        using iterator = base_iterator<false>;
        using const_iterator = base_iterator<true>;

        forward_list() noexcept
        : sz(0), end_{&end_} {}

        iterator begin() {
            return iterator(end_.next);
        }

        iterator end() {
            return iterator(&end_);
        }

        const_iterator begin() const {
            return const_iterator(end_.next);
        }

        const_iterator end() const {
            return const_iterator(&end_);
        }

        //Константные итераторы для неконстантного контейнера
        const_iterator cbegin() const {
            return const_iterator(end_.next);
        }

        const_iterator cend() const {
            return const_iterator(&end_);
        }

        // void insert_after(const T& value, const size_t pos) {
        //     //size == 0 нужно потому, что когда sz == 0, end совпадает с begin
        //     //При попытке вставить в stl forward_list с размером 0 элемент после begin iterator,
        //     //получим RT ошибку segmentation fault
        //     if (pos > sz || sz == 0) {
        //         return;
        //     } 

        //     Node* newnode = reserve(value);
        //     Node* node = getNode(pos);
        //     newnode->next = node->next;
        //     node->next = newnode->next;
        // }

        //Функция insert_after/push_front является сильной относительно исключений, т.е
        //при ее неудачном вызове все должно вернуться в то же состояние, 
        //что и до ее вызова
        //новая функция для вставки после итератора. Оставим предыдущую как легаси
        void insert_after(const T& value, iterator pos) {
            Node* newnode = reserve(value);
            newnode->next = pos.ptr->next;
            pos.ptr->next = newnode;
        }

        void erase_after(iterator pos) {
            Node* deletenode = static_cast<Node*>(pos.ptr->next); //Сохраняем указатель
            pos.ptr->next = deletenode->next;
            sz--;
            operator delete(deletenode);
        }

        void push_front(const T& value) {
            Node* newnode = reserve(value);
            newnode->next = end_.next;
            end_.next = newnode;
        }

        void pop_front() {
            Node* deletenode = static_cast<Node*>(begin().ptr);
            end_.next = deletenode->next;
            sz--;
            operator delete(deletenode);
        }

        iterator find(const T& value) {
            for (auto it = begin(); it != end(); it++) {
                if (*it == value) {
                    return it;
                }
            }
            return end();
        }

        [[nodiscard]]
        size_t size() const noexcept {
            return sz;
        }

        [[nodiscard]]
        bool empty() const noexcept {
            return sz == 0;
        }

    private:

        Node* reserve(const T& value) {
            //Избегаем проблему того, что у T нет конструктора по умолчанию
            Node* newnode = static_cast<Node*>(operator new(sizeof(Node)));
            try {
                //Копирование в данном случае является UB(в выделенной памяти ничего не лежит),
                //поэтому используем placement new
                new(&newnode->value) T(value);
            } catch(...) {
                //Сам placement new исключения не кидает, т.к памяти он не запрашивает, но
                //T кинуть исключение может
                //Может быть перед operator delete стоит вызвать деструктор у T, т.к я так и не 
                //выяснил, вызывает ли он деструкторы(но похоже что нет)
                operator delete(newnode);
                throw;
            }
            sz++;
            return newnode;
        }
        //Осталась со времен листа без итератора
        Node* getNode(const size_t pos) noexcept {
            Node* node = end_.next; //Begin node
            for (size_t index = 0; index < pos; index++) {
                node = node->next;
            }
            return node;
        }

    };
};

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
    l.pop_front();
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
    return 0;
}