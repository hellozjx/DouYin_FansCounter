<?php
function curl_get($url, $header=array(),$data = array())
{
	//初始化
	$ch = curl_init();
	//设置选项，包括URL
	$query = http_build_query($data);
	$url = $url . '?' . $query;
	curl_setopt($ch, CURLOPT_URL, $url);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_HEADER, true);
	curl_setopt($ch, CURLOPT_TIMEOUT, 10);
	if(!empty($header))
	{
		curl_setopt($ch, CURLOPT_HTTPHEADER, $header);
	}
	//执行并获取HTML文档内容
	$output = curl_exec($ch);
	
	$headerSize = curl_getinfo($ch, CURLINFO_HEADER_SIZE);
	// 根据头大小去获取头信息内容
	$rheader = substr($output, 0, $headerSize);
	$body = substr($output, $headerSize);
	//释放curl句柄
	curl_close($ch);
	$result = json_decode($body, true);
	if($rheader){
		preg_match_all("/set\-cookie:([^;]*);/", $rheader, $matches);
		$cookie  = substr($matches[1][0], 1);
		file_put_contents("cookie.txt",$cookie."\n",FILE_APPEND);
		//$result["response_cookie"]=$cookie;
	}
	return $result;
}
function get_data()
{
    $url="https://creator.douyin.com/web/api/media/user/info/";
    $headers=array(
    'cookie: 这里输入你登录抖音创作平台后的cookies',
    'user-agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Safari/537.36',
    'accept-language: zh-CN,zh;q=0.9',
    ':authority: creator.douyin.com',
    ':path: /web/api/media/user/info/'
    );
    $info=curl_get($url,$headers);
    return $info;
}
//因为发现不是每次都能正确取到值，所以加入redis做缓存。
$redis = new Redis();
$redis->connect('127.0.0.1', 6379);
$user_info = get_data();
$follower_count = $redis->get("follower_count");
$total_favorited = $redis->get("total_favorited");

if(intval($user_info[user][follower_count])>$follower_count)
{
    $redis->set("follower_count",$user_info[user][follower_count]);
}
if(intval($user_info[user][total_favorited])>$total_favorited)
{
    $redis->set("total_favorited",$user_info[user][total_favorited]);
}
$follower_count = $redis->get("follower_count");
$total_favorited = $redis->get("total_favorited");
$data['code']=0;
$data['message']="success";
$data['ttl']=1;
$data['data']=array('mid'=>1,'following'=>0,'favorited'=>intval($total_favorited),'black'=>0,'follower'=>intval($follower_count));
$json=json_encode($data);
header('Content-Type: application/json; charset=utf-8');
echo $json;

?>