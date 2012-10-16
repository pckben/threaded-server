PORT=$1
for x in `seq 0 9`; do
	for x in `seq 0 9`; do
		#echo $x$x$x$x$x$x$x$x$x$x
		bin/test_client 127.0.0.1 $PORT $x$x$x$x$x$x$x$x$x$x &
	done
done
