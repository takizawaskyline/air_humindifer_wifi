1. Увлажнитель на ESP32. Управляется через Telegram. Может работать в трех режимах: ручном (включаешь и выключаешь сам), автоматическом (он сам решает, когда включаться) и настраиваемом (держит влажность, которую ты задал). Показывает температуру, влажность и давление. Рисует графики, как менялись показатели. Если давление падает – говорит, что погода ухудшится. Настройки сохраняет, даже если вырубить питание. Подключается к Wi-Fi, если отвалится – перезагрузится сам. В общем, включаешь автоматический режим и забываешь.


2. Создание бота
Полная инструкция есть на сайте хабр в этой статьи https://habr.com/ru/articles/262247/ тут я привел из нее самое главное
Прежде чем начинать разработку, бота необходимо зарегистрировать и получить его уникальный id, являющийся одновременно и токеном. Для этого в Telegram существует специальный бот — BotFather.

Пишем ему /start и получаем список всех его команд.
Первая и главная — /newbot — отправляем ему и бот просит придумать имя нашему новому боту. Единственное ограничение на имя — оно должно оканчиваться на «bot». В случае успеха BotFather возвращает токен бота и ссылку для быстрого добавления бота в контакты, иначе придется поломать голову над именем.

Для начала работы этого уже достаточно. Взято со статьи на хаборе (https://habr.com/ru/articles/262247/)

3. Закупка компонентов
   Esp32 +- 300р (https://aliexpress.ru/item/1005008644023249.html?sku_id=12000046073762862&spm=a2g2w.productlist.search_results.2.2dd54037R84wJ4)
   
   Реле +- 50р (https://aliexpress.ru/item/1005007430142528.html?sku_id=12000040725647432&spm=a2g2w.productlist.search_results.1.35fc6eebXY8sTV)
   
   bmp180 +- 50р (https://aliexpress.ru/item/1005007932330911.html?sku_id=12000042920153584&spm=a2g2w.productlist.search_results.0.5df95baflaGzrH)
   
   dht11 или dht22 +-50р советую брать без модуля просто голый датччик (https://aliexpress.ru/item/1005008593539984.htmlsku_id=12000045873045875&spm=a2g2w.productlist.search_results.3.24074078fZHCaw)

   Пожалуй самое дорогое это сам увлажнитель можно найти вариант как и за 100 рублей так и за 3000 рублей. Главное чтобы в нем было много места внутри подо все модули.
   




Схема подключения:


![image](https://github.com/user-attachments/assets/17488b49-b213-48f0-9719-9811b3e67891)


Через 1 и 2 контакт реле нужно разорвать питание на ультразвуковой динамик увлажнителя как показано ниже 


![1 (1)](https://github.com/user-attachments/assets/1e949bee-fd39-4e4e-9a2e-bafc937c7eca)
