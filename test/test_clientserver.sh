SERVER=./test_server
CLIENT=./test_client
THREADS=4
N=10
PORT=12345

rm -f $SERVER.log
rm -f $CLIENT.log
rm -f $CLIENT.pid

echo "starting server..."
$SERVER $THREADS $PORT > $SERVER.log &
SPID=$!
echo $SPID
sleep 1

echo "starting $N clients..."
for (( x=0; x<$N; x++ )); do
  $CLIENT 127.0.0.1 $PORT $x$x$x$x$x$x$x$x$x$x >> $CLIENT.log &
  echo $! >> $CLIENT.pid
done

echo "waiting for clients..."

for x in `cat $CLIENT.pid`; do
  wait $x
done

ok=true
for (( x=0; x<$N; x++ )); do
  s=$x$x$x$x$x$x$x$x$x$x
  if ! (grep -Fxq "Sent: $s" $CLIENT.log && 
        grep -Fxq "Received: $s" $CLIENT.log && 
        grep -Fxq "Sent: $s" $SERVER.log && 
        grep -Fxq "Received: $s" $SERVER.log)
  then
    ok=false
    break
  fi
done

echo "killing server..."
kill $SPID
wait $SPID 2>/dev/null

#rm -f $SERVER.log
#rm -f $CLIENT.log
rm -f $CLIENT.pid

if $ok; then
  echo "OK"
else
  echo "Failed."
  exit -1
fi
