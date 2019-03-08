ssh n2 "cd rpc; make test" & 
ssh n3 "cd rpc; make test"  &
ssh n4 "cd rpc; make test"  &
ssh n5 "cd rpc; make test"  &
sleep 1
killall ssh
