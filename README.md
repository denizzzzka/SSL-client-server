# SSL-client-server
Данный код реализует защищенное соединение между клиентом и сервером с использованием библиотеки openssl. При получении сообщения сервер пишет его заглавными буквами и отправляет назад. Также в процессе работы программы генерируется файл Keys.txt. В нем хранятся ключи, котрые можно использовать для декодирования. Для корректной работы программы также нужно создать сертификат.
# Создание сертификата
Сертфикат создается так :  
`openssl req -x509 -nodes -days 365 -newkey rsa:1024 -keyout mycert.pem -out mycert.pem`
