
## Setup DEV environment

> Tested with php8.3 and php8.4

1. Install for Ubuntu:

    ```bash
    sudo apt install nginx-full php8.4-xml php8.4-cli php8.4-dev build-essential php8.4-mbstring libsodium-dev libzstd-dev redis-server redis-tools
    ```

2. Build and install [HIREDIS](https://github.com/redis/hiredis) from source:

    ```bash
    cd $HOME/Documents
    git clone --branch v1.3.0 https://github.com/redis/hiredis.git
    cd hiredis
    make clean && make && sudo make install
    ```

3. Build and install [Phpiredis](https://github.com/nrk/phpiredis) from source:

    ```bash
    cd $HOME/Documents
    git clone --branch v1.1 https://github.com/nrk/phpiredis.git
    cd phpiredis
    phpize && ./configure && make clean && make && sudo make install
    ```

4. Build and install [libsodium-php](https://github.com/jedisct1/libsodium-php) from source:

    ```bash
    cd $HOME/Documents
    git clone https://github.com/jedisct1/libsodium-php.git
    cd libsodium-php
    phpize && ./configure && make clean && make && sudo make install
    ```

5. Build and install [Zstd Extension for PHP](https://github.com/kjdev/php-ext-zstd) from source:

    ```bash
    cd $HOME/Documents
    git clone https://github.com/kjdev/php-ext-zstd.git
    cd php-ext-zstd
    phpize && ./configure --with-libzstd && make clean && make && sudo make install
    ```

    > For debian 12: sudo pecl install -f libsodium

5. Enable extensions for php cli on Ubuntu:

    ```bash
    cat /etc/php/8.3/cli/conf.d/30-lowresrmx.ini
    ```
    ```ini
    extension=mbstring.so
    extension=phpiredis.so
    extension=sodium.so
    extension=zstd.so
    extension=xml.so
    ```

7. Check for loaded extensions:

    ```bash
    php -i | grep mbstring
    php -i | grep redis
    php -i | grep sodium
    php -i | grep zstd
    php -i | grep xml
    ```

Create and fill the `sources/private.php` file using the given `private.sample.php`

8. Start the server:

    ```bash
    cd sources
    php -S 0.0.0.0:8080 index.php
    ```

9. Add to your `/etc/hosts`:

    ```hosts
    127.0.0.1 lowresrmx.top
    ```

## Create a fake session

```
redis-cli hmset s:infdev status allowed uid 123 csrf 123
redis-cli rpush u:123:s infdev
redis-cli hmset u:123 name infdev picture "" locale en author infdev
```

Create a cookie sid with 696e66646576 inside
