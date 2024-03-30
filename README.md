# SSL-client-server
Данный код реализует защищенное соединение между клиентом и сервером с использованием библиотеки openssl. При получении сообщения сервер пишет его заглавными буквами и отправляет назад. Также в процессе работы программы генерируется файл Keys.txt. В нем хранятся ключи, котрые можно использовать для декодирования. Для корректной работы программы также нужно создать сертификат.
# Создание сертификата
Сертфикат создается так :  
`openssl req -x509 -noenc -days 30 -newkey rsa -keyout mycert.pem -out mycert.pem`

-x509 - создает "учебный" сертификат  
-noenc - приватный ключ не зашифрован  
-days - время жизни, используется из-за флага -x509  
-newkey rsa - новый ключ, для генерации используется rsa, используется из-за флага -x509  
-keyout mycert.pem -out mycert.pem - куда сохраняем  
# Компиляция  
` gcc -o main main.c -L/usr/lib -lssl -lcrypto`  
# Присер файла Keys.txt  
![image](https://github.com/denizzzzka/SSL-client-server/assets/91347518/9f3a2bec-5c2c-4753-852d-74aab5e19ab5)  
# Пример зашифрованных сообжений и дешифровки  
Дешифровка выполняется с помощью сертификата и файла Keys.txt 


