#транспортный справочник
Проект был создан во время обучения и сейчас находится в доработка. Планируется создание GUI на QT.
Сам проект представляет из себя обработчки кратчайших маршрутов по заготовленной базе. В итоге получаем в виде JSON файла кратчайший путь и карту передвижения.


для установки:
* создайте папку build и из нее выполните команды:
1. mkdir build
2. cd build 
3. cmake <путь к проекту> -DCMAKE_PREFIX_PATH=<путь к Protobuf>
4. cmake --build .

* для проверки есть заготовки примеров в папку examples.
1. Запуская из командной строки добавляем один из аргументов - первый раз make_base - программа будет ожидать для принятия данные базы, они находятся в файлах с пометкой make_base.
2. Второй раз process_requests и передаем данные из файла с аналогичным названием. 
3. Если все правильно, то программа выведет данные в формате JSON - один из пунктов будет Map в формате svg - его можно скопировать, убрать все обратные слешы в редакторе и создать отдельный фалй в формате SVG (.svg). При открытии его будет отображена карта маршрутов. 
