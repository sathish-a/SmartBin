<?php $root_url = 'https://smartbin-7184b.firebaseio.com/test/ip.json?print=pretty';

$ch = curl_init();
	curl_setopt_array($ch, array(
    CURLOPT_RETURNTRANSFER => 1,
    CURLOPT_URL => $root_url,
	));
	$resp = curl_exec($ch);
	curl_close($ch);
	echo "{\"response\":".trim($resp)."}";
?>