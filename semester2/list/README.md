# Задача "Лист"

В этой задаче вам необходимо реализовать шаблонный класс List\<T, Allocator\> аналог std::list из stl.

Реализация должна быть основана на нодах (класс Node) которые содержат указатели на лево и на право.

Ваш List должен уметь в:

* Базовую функциональность + итераторы
* Поддержку аллокаторов
* Быть exception-safety

## Codestyle

Все методы snake_case для соответствия stl'ному list'у

## Методы:

### Using'и

* value_type
* allocator_type
* iterator (должен удовлетворять https://en.cppreference.com/w/cpp/named_req/BidirectionalIterator)

### Конструкторы

Ваш класс должен реализовывать следующие конструкторы (что они делают можно посмотреть на cppref):

* List()
* List(size_t count, const T& value = T(), const Allocator& alloc = Allocator())
* explicit List(size_t count, const Allocator& alloc = Allocator())
* List(const list& other);
* List(std::initializer_list\<T\> init, const Allocator& alloc = Allocator())

### Деструктор

Ну, он должен быть :)

### Iterators (с поддержкой константных)

* begin()
* end()
* cbegin()
* cend()

### operator=

* List& operator=(const List& other)

***Note*** Важное уточнение по поводу оператора присваивания: вы должны пользоваться propagate_on_container_copy_assignment. Это будет проверяться в тестах. Как это работает написано на cppref https://en.cppreference.com/w/cpp/container/list/operator%3D

### element access methods

* T& front()
* const T& front() const
* T& back()
* const T& back() const


### Capacity

* bool empty()
* size_t size()

### Modifiers

* push_back(front)(const T&)
* push_back(front)(T&&)
* pop_back(front)();

## Различные требования:

### Поддержка аллокаторов

* Вы должны работать с памятью только через аллокатор
* Вы должны конструировать и разрушать объекты только через аллокатор
* Вы должны поддержать propagate_on_container_copy в соответствующих методах
* Вы должны использовать rebind для аллоцирования и конструирования внутреннего класса Node

### Exception-safety

Общая концепция: если где-то выскочит исключение ваш контейнер должен вернуться в оригинальное состояние и пробросить исключение наверх.

Т.е. ваш класс должен быть готов к исключениям в копировании объекта T и в конструкторах объекта T. (исключений в мув конструкторах не будет, про это думать не надо)