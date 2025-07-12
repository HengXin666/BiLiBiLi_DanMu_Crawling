#include <ylt/struct_pack.hpp>
#include <ylt/struct_pb.hpp>
#include <ylt/coro_http/coro_http_client.hpp>

#include <pojo/BasDanMaKu.hpp>
#include <pojo/DanMaKu.hpp>

#include <HXLibs/log/Log.hpp>

using namespace HX;

int main() {

/*
    获取BAS弹幕专包
    url = f'https://api.bilibili.com/x/v2/dm/web/view?type=1&oid={cid}'
    data = requests.get(url, cookies=ReqDataSingleton().getAnyOneCookies(), headers=ReqDataSingleton().UserAgent, timeout=10)
*/
    cinatra::coro_http_client cli{};
    
    uint64_t cid = 1176840;

    auto data = cli.get(
        "http://i0.hdslb.com/bfs/dm/09cdc8240413e26f174d9342bf6e41480c326873.bin"
    );
    
    log::hxLog.info(data.status, data.resp_body);

    HX::DmSegMobileReply bas;

    struct_pb::from_pb(bas, data.resp_body);

    log::hxLog.info(bas);

    return 0;
}