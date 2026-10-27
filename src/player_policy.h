#pragma once
#include <QUrl>
#include <QUrlQuery>
inline constexpr const char* PlayerPageUrl="https://pexy.io/game/sonic-the-hedgehog-2-tiger-handheld/";
inline constexpr const char* PlayerEmbedUrl="https://pexy.io/wp-content/emus/lcd/index.html?driver=tsonic2&game=sonic-the-hedgehog-2-tiger-handheld%2Fsonic-the-hedgehog-2-tiger-handheld.zip&art=1&cv=1791774870&v=1791789791";
inline bool playerRequestAllowed(const QUrl& url){
 if(url.scheme()=="data"||url.scheme()=="blob"||url==QUrl("about:blank"))return true;
 if(url.scheme()!="https"||url.host()!="pexy.io"||!url.userInfo().isEmpty()||(url.port()!=-1&&url.port()!=443))return false;
 const auto path=url.path();
 return path.startsWith("/wp-content/emus/lcd/")||path.startsWith("/wp-content/emus/arcade/cores/handheld/")||
  path.startsWith("/wp-content/games/lcd/sonic-the-hedgehog-2-tiger-handheld/");
}
inline bool playerNavigationAllowed(const QUrl& url){
 QUrlQuery q(url);
 return playerRequestAllowed(url)&&url.path()=="/wp-content/emus/lcd/index.html"&&q.queryItemValue("driver")=="tsonic2"&&
  q.queryItemValue("game",QUrl::FullyDecoded)=="sonic-the-hedgehog-2-tiger-handheld/sonic-the-hedgehog-2-tiger-handheld.zip";
}
