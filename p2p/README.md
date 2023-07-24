# P2P
### Стек: C/C++, eosio.cdt 1.7.0

# Введение
Контракт обеспечивает обмен токенами между двумя пользователями системы с участием гаранта. Создавая ордер типа sell, токены продавца блокируются в контракте и передаются покупателю после подтверждения завершения сделки. 

Каждый ордер, не обладающий родителем, готов принимать предложения о сделках в дочерних ордерах противоположного типа. Так, если родительский ордер типа buy, то он готов принимать предложения от ордеров с типом sell, и наоборот. Обмен производится по курсу относительно системной опорной валюты (USD). 

В процессе обмена всегда задействованы три типа валют:
- root_asset - токен обмена, который предоставляет продавец для блокировки в контракте и получает покупатель после завершения сделки.
- quote_asset - опорный токен, который используется для расчёта количества валюты выхода.
- out_asset - токен выхода, который получает продавец и отправляет покупатель на реквизиты продавца. 


# Компиляция
Заменить ABSOLUTE_PATH_TO_CONTRACT на абсолютный путь к директории контракта p2p. 
```
docker run --rm --name eosio.cdt_v1.7.0 --volume /ABSOLUTE_PATH_TO_CONTRACT:/project -w /project eostudio/eosio.cdt:v1.7.0 /bin/bash -c "eosio-cpp -abigen -I include -R include -contract p2p -o p2p.wasm p2p.cpp" &
```


# Генерация документации
Требуется Doxygen версии от 1.9.3
```
git submodule update --init --recursive
doxygen
```


# Установка
```
cleos set contract p2p ABSOLUTE_PATH_TO_CONTRACT -p p2p
```


# Применение
Контракт может использоваться:
- как p2p биржа обмена любым стандартным заменяемым цифровым активом;
- как точка реализации токена компанией (IDO), которая не исключает возможность перепродаж токенов пользователями;
- как касса взаимопомощи, где только те пользователи могут получить валюту выхода на свои реквизиты, которые обладают токенами обмена;
Во всех вариантах каждый хост цифровой экономики на аккаунте _core может создать дополнительный стимул на продаже собственных токенов, активировав партнёрскую программу. Для этого, он должен пополнить бонусный баланс контракта и установить бонусный курс, который будет использоваться при расчёте вознаграждений по структуре покупателя по формуле:
```
КОЛИЧЕСТВО_ТОКЕНОВ_ПАРТНЁРАМ_ПОКУПАТЕЛЯ = КОЛИЧЕСТВО_ТОКЕНОВ_В_УСПЕШНОЙ_СДЕЛКЕ_У_ПРОДАВЦА * БОНУСНЫЙ_КУРС_ПРОДАВЦА
```

# Действующие аккаунты
me - собственное имя контракта;
curator - дефолтный оракул, используемый во всех сделках;
rater - имя аккаунта поставщика курсов обмена;
core - имя аккаунта, распределяющего партнерское вознаграждение;


# Основные управляющие параметры
ENABLE_GROWHT = TRUE - допускает изменение курса системного токена в процентном соотношении от стартового курса (линейный рост)
START_RATE - стартовый курс системного токена, используемый при расчёте линейного роста;
PERCENTS_PER_MONTH - устанавливает скорость роста системного токена;
ENABLE_VESTING - включает режим продаж через вестинг равными долями от момента создания объекта;
VESTING_SECONDS - продолжительность вестинга;
CORE_SALE_ACCOUNT - аккаунт компании, осуществляющий IDO, продажа от которого проходит с применением вестинга. Все остальные аккаунты совершают сделки без вестинга;
GIFT_ACCOUNT_FROM_AMOUNT - если покупка совершается у компании на сумму, превышающую в этом поле, то компания совершает выкуп прав владельца аккаунта у регистратора и передаёт их пользователю (переводит аккаунт из статуса гость в статус партнёр).
 
# Роли
- Продавец - создаёт ордер типа sell
- Покупатель - создаёт ордер типа buy
- Оракул - разрешает споры между партнёрами
- Поставщик курса - поставляет курс валют выхода
- Системный поставщик курса - поставляет курс системной валюты

# Сценарии
### Пополнение собственного баланса
Для того, чтобы иметь возможность создать ордер на продажу, необходимо обладать балансом на контракте. Для этого, необходимо совершить перевод на имя аккаунта контракта без дополнительных параметров в поле memo:
```
cleos transfer username p2p "100.0000 FLOWER"
```
Проверить собственный баланс с помощью команды:
```
cleos get table p2p username balance
```
Ответ содержит имя контракта и количество токенов, доступных к сделке продажи:
```
"rows": [{
      "id": 0,
      "contract": "eosio.token",
      "quantity": "100.0000 FLOWER"
    }
  ],
```

### Пополнение бонусного баланса
Для пополнения бонусного баланса необходимо совершить перевод на собственное имя аккаунта контракта с указанием в поле memo имени аккаунта, который активирует бонусную систему. Бонусная система доступна только хостам цифровых экономик в контракте core. Аккаунты, не обладающие статусом хоста в контракте core не смогут распределить бонусные токены на структуру покупателя.

```
cleos transfer username p2p "100.0000 FLOWER" "username"
```

Проверить бонусный баланс с помощью команды:
```
cleos get table p2p p2p bbonuses
```
Ответ содержит имена аккаунтов хостов и их бонусные балансы:
```
"rows": [{
      "host": "alexant",
      "contract": "eosio.token",
      "total": "200.0000 FLOWER",
      "available": "200.0000 FLOWER",
      "distributed": "0.0000 FLOWER",
      "distribution_rate": "0.00000000000000000"
    },{
      "host": "core",
      "contract": "eosio.token",
      "total": "10000.0000 FLOWER",
      "available": "9900.0000 FLOWER",
      "distributed": "100.0000 FLOWER",
      "distribution_rate": "1.00000000000000000"
    }
  ],
```
При повторном пополнении бонусного баланса, соответствующие токены суммируются. 

### Активация бонусной системы
Для активации бонусной системы, аккаунт должен обладать статусом активного хоста в контракте core с установленными параметрами распределения по уровням пользователей. Для того, чтобы в момент продажи бонусные токены распределились среди партнёров покупателя, необходимо установить бонусный курс распределения:
```
cleos push action p2p setbrate '[username, 1]' -p username 
```
Где 1 - это бонусный курс, который распределяет на партнёров покупателя в точности ту сумму (если она доступна в бонусном балансе хоста), которая находилась в сделке. При установке 0 - распределение бонусных токенов останавливается. 


### Продажа
Создавая ордер на продажу, продавец указывает валюту выхода, которую намерен получить от покупателя и курс к опорной валюте (USD), по которому готов осуществить продажу, а также реквизиты для получения валюты выхода. В момент создания ордера на продажу, контракт блокирует токены продавца, после чего, вывести их из блокировки можно только в случае отмены ордера (метод cancel), завершения сделки (метод approve), или разрешения открытого спора. После создания ордера, продавец ожидает встречного ордера от покупателя. 

Покупатель создаёт встречный ордер, фиксируя курс валюты выхода на этот момент. После получения встречного ордера, продавец должен подтвердить своё присутствие и условия обмена. Встречный курс обмена в дочернем ордере может отличаться от того, что был заявлен продавцом. 

Если встречные условия продавца устраивают, то он подтверждает начало сделки методом accept с передачей зашифрованных реквизитов для перевода валюты выхода покупателю. После вызова метода accept продавцом, статус дочернего ордера меняется на process. Статус родительского ордера продавца находится в waiting и не изменяется до момента полной реализации токенов. 

Теперь, покупатель должен осуществить перевод валюты выхода на реквизиты продавца. Совершив перевод, продавец подтверждает факт отправки средств вызовом метода confirm с указанием идентификатора своего ордера, что меняет статус ордера на payed. Теперь, продавец должен подтвердить факт получения валюты выхода на свои реквизиты, вызовом метода approve с указанием идентификатором встречного ордера. При получении транзакции с действием approve от продавца, происходит разблокировка токенов обмена в пользу покупателя. 

### Покупка
Создавая ордер на покупку, покупатель указывает валюту выхода, которую намер получить от продавца и курс к опорной валюте (USD), по которому готов осуществить покупку. После создания ордера на покупку, покупатель ожидает появления встречного ордера от продавца. 

Продавец создаёт встречный ордер, фиксируя курс валюты выхода на этот момент и передаёт зашифрованные реквизиты покупателю. Контракт, при этом, блокирует его токены обмена до момента завершения сделки (метод approve), отмены (метод cancel) или разрешения открытого спора. 

Если встречные условия устраивают покупателя, то он подтверждает начало сделки методом accept, после чего, статус дочернего ордера меняется на process. Статус родительского ордера покупателя находится в waiting и не изменяется до момента реализации токенов. 

Теперь, покупатель должен осуществить передачу валюты выхода на реквизиты, предоставленные продавцом. После передачи, покупатель подтверждает этот факт вызовом метода confirm, а дочерний ордер переходит в статус payed, статус родительского ордера при этом не изменяется. 

После получения оплаты в валюте выхода на свои реквизиты, покупатель должен вызвать метод approve с указанием идентификатора встречного ордера. При получении транзакции с подтверждением о получении валюты выхода от продавца, контракт разблокирует токены обмена в пользу покупателя. 


### Разрешение споров
TODO реализовать метод и логику разрешения споров. 


### Частичный обмен
Количество токенов на обмене, которые хочет получить покупатель, могут отличаться в меньшую сторону от заявки продавца. В этом случае, произойдёт срабатывание ордера только на сумму обмена, а остаток останется в ордере продавца для обмена с другими покупателями. 

### Обновление опорных курсов
Контракт содержит опорные курсы конвертации валют выхода к опорной валюте (USD), которые поставляются специальным аккаунтом поставщика из внешних источников (rater). 

```
cleos push action p2p setrate '["", "0.0000 RUB", 0.0133]' -p rater
```
Для получения курсов валют выхода вызываем команду:
```
cleos get table p2p p2p usdrates
```
В ответе увидим:
```
"rows": [{
      "id": 0,
      "out_contract": "",
      "out_asset": "0.0000 RUB",
      "rate": "0.01330000000000000",
      "updated_at": "2021-12-30T13:34:31"
    }]
```
Где 0.0133 - это курс RUB / USD, что соответствует 75,1879 USD / RUB. ВАЖНО! Обращаем внимание, что курсы поставляются в базе к USD, что часто соответствует обратным значением к тем, которые привычны. 

TODO На текущий момент, поставщик курсов один, в дальнейшем реализовать скользящую среднюю курсов на 14 дней от 21 делегата. 


### Обновление курса системного токена
Обновление курс системного токена производится системным аккаунтом eosio раз в минуту, если этот режим активирован с его стороны. В любом случае, для того, чтобы изменить курс реализации системного токена на продаже, необходимо вызвать метод uprate:
```
cleos push action p2p uprate '["eosio.token", "0.0000 FLOWER"]' -p eosio
```
Расчёт изменений курса будет отображен в таблице usdrates. Курс обмена увеличится на линейную величину в соотстветствии с процентом роста PERCENTS_PER_MONTH и стартовым курсом START_RATE. 

Системный курс используется в веб-терминале при активированном режиме продаж системного токена, что отключает для пользователей визуальную возможность редактирования курса обмена, а все обмены производятся по установленному системному курсу на момент получения встречного ордера.


### Шифрование реквизитов
Реквизиты передаются через блокчейн в зашифрованном виде по протоколу Диффи-Хеллмана, при котором, шифрование происходит с участием приватного ключа отправителя и публичного ключа получателя. Таким образом, только им двоим доступны зашифрованные реквизиты, поскольку расшифровка доступна как с помощью приватного ключа отправителя, так и с помощью приватного ключа получателя. 

### Документация к методам и таблицам контракта
Документация к методам и таблицам контракта доступна в папке docs/html/index.html