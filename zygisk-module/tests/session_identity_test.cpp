#include "../cpp/session_identity.h"
#include <cassert>
#include <cstdio>
int main(){
    using pikmin::account_digest;
    assert(account_digest("abc")=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    assert(account_digest(std::string(100,'a'))=="2816597888e4a0d3a36b82b83316ab32680eb8f00f8cd3b904d681246d285a0e");
    assert(account_digest("").empty()&&account_digest(std::string(513,'a')).empty());
    assert(account_digest("a/b+c=").size()==64);
    assert(account_digest(std::string("a\0b\0",4)).size()==64);
    assert(account_digest("player-A")!=account_digest("player-B"));
    puts("PASS account digest vectors, bounds and distinct identities; NOT login hook verification");
}
