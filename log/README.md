# LOG
### Стек: C/C++, eosio.cdt 1.5.0

# Введение
Контракт используется для создания постов и комментариев к ним неограниченной вложенности. Работает в связке с модулем DEMUX, получающим посты из блокчейна и модулем GRAPHQL для выдачи их веб-терминалу по API. На текущий момент хранит посты в оперативной памяти, что не является обязательным. 

# Компиляция
Заменить ABSOLUTE_PATH_TO_CONTRACT на абсолютный путь к директории контракта log. 
```
docker run --rm --name eosio.cdt_v1.5.0 --volume /ABSOLUTE_PATH_TO_CONTRACT:/project -w /project eostudio/eosio.cdt:v1.5.0 /bin/bash -c "eosio-cpp -abigen -I include -R include -contract log -o log.wasm log.cpp" &
```


# Генерация документации
Требуется Doxygen версии от 1.9.3
```
git submodule update --init --recursive
doxygen
```


# Установка
```
cleos set contract log ABSOLUTE_PATH_TO_CONTRACT -p log
```

# Действующие аккаунты
- me - собственное имя контракта;


# Основные управляющие параметры
- MIN_PERMLINK_LENGTH - минимальное количество символов в прямой ссылке
- MAX_PERMLINK_LENGTH - максимальное количество символов в прямой ссылке

# Роли
- Автор - создаёт и удаляет пост

# Сценарии
### Создать пост
Для создания поста, необходимо вызвать метод post:
```
cleos push action log post '[tester, tester, "permlink", "", "", "title", "body", "", true, 0, false, ""]' -p tester
```

Для создания ответа на этот пост, необходимо вызвать метод post с указанием параметров поста-родителя:
```
cleos push action log post '[tester, tester, "permlink2", "permlink", "tester", "title", "body", "", true, 0, false, ""]' -p tester
```


### Удалить пост
Для удаления поста необходимо вызвать метод del: 
```
cleos push action log del '[tester, tester, "permlink"]' -p tester
```



### Документация к методам и таблицам контракта
Документация к методам и таблицам контракта доступна в папке docs/html/index.html


# TODO