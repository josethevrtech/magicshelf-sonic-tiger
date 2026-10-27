#include "player_policy.h"
#include <cassert>
int main(){
 assert(playerNavigationAllowed(QUrl(PlayerEmbedUrl)));
 assert(playerRequestAllowed(QUrl("https://pexy.io/wp-content/emus/lcd/lcd.js")));
 for(const auto* url:{"http://pexy.io/wp-content/emus/lcd/lcd.js","https://evil.test/wp-content/emus/lcd/lcd.js","https://pexy.io.evil.test/wp-content/emus/lcd/lcd.js","https://pexy.io:444/wp-content/emus/lcd/lcd.js","https://pexy.io/wp-content/other/file","https://static.cloudflareinsights.com/beacon.min.js"})assert(!playerRequestAllowed(QUrl(url)));
 QUrl other(PlayerEmbedUrl);QUrlQuery q(other);q.removeAllQueryItems("driver");q.addQueryItem("driver","tsonic");other.setQuery(q);assert(!playerNavigationAllowed(other));
}
