PORT=$1
bin/test_client 127.0.0.1 $PORT 0000000000 &
bin/test_client 127.0.0.1 $PORT 1111111111 &
bin/test_client 127.0.0.1 $PORT 2222222222 &
bin/test_client 127.0.0.1 $PORT 3333333333 &
bin/test_client 127.0.0.1 $PORT 4444444444 &
