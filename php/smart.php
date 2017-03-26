<?php

//https://sathish97a.000webhostapp.com/smart.php
//https://sathish97a.000webhostapp.com/test.php

/* 	 METHODS         DESCRIPTION       		PARAMETERS TO PASS  		  RESPONSE
____________________________________________________________________________________________________________________

			ADD_BIN 		To add new bin   		       Level  						    New Id of Dustbin , 1(Error)

			UPDT_LEV		To update the level        BinId , Level  				100(SUCCESS) , 1(ERROR)
							    of existing bin

			UPDT_LOC		To Update the location     BinId, Lat , Lng   		100(SUCCESS) , 1(ERROR)
							    of existing bin
_____________________________________________________________________________________________________________________
*/

$root_url = 'https://smartbin-7184b.firebaseio.com';
$bin_url = $root_url."/bins";
$bin_level = "/level";

$S_LAT = 'lat';
$S_LON = 'lng';
$S_LOC = 'location' ;
$S_VAL = "level";
$S_FUN = "function";
$S_ID = "binid";

if(isset($_POST))
{	if(isset($_POST["function"]))
	{
		$__FUN = $_POST["function"];

		if(strcmp($__FUN, "UPDT_LEV") == 0)
			{
				if(isset($_POST["binid"]) && isset($_POST["level"]))
					{
						updateLevel($_POST["binid"],$_POST["level"]);
					}

			}

		if(strcmp($__FUN, "ADD_BIN") == 0)
			{
				if(isset($_POST["level"]))
				{
					addNewBin($_POST["level"]);
				}
			}

		if(strcmp($__FUN, "UPDT_LOC") == 0)
		{
			if(isset($_POST["binid"]) && isset($_POST["lat"]) && isset($_POST["lng"]))
			{
				updateLocation($_POST["binid"],$_POST["lat"],$_POST["lng"]);
			}
		}
	}

}


function addNewBin($val)
{
	$myUrl = $GLOBALS['bin_url'].".json";
    $data = json_encode( array( $GLOBALS['S_VAL'] => $val , "timestamp" => array(".sv" => "timestamp" ) , $GLOBALS['S_LOC'] => array( $GLOBALS['S_LAT'] => "1" , $GLOBALS['S_LON'] => "1") )  );
    $ch = curl_init();
    curl_setopt_array($ch, array(
    CURLOPT_RETURNTRANSFER => 1,
    CURLOPT_URL => $myUrl,
    CURLOPT_POST => 1,
    CURLOPT_POSTFIELDS => $data));
	$resp = curl_exec($ch);
	curl_close($ch);
	//echo $resp;
	$data = json_decode($resp);
	$name =  $data -> {'name'};

	if($name) $data = array('response' => $name);
	else $data = array('response' => '1');
    echo json_encode($data);
}


function updateLevel($binid ,$val)
{

	$myUrl = $GLOBALS['bin_url']."/{$binid}.json";
	$data = json_encode( array( $GLOBALS['S_VAL'] => $val , "timestamp" => array(".sv" => "timestamp" ) ) );
	$ch = curl_init();
	curl_setopt_array($ch, array(
    CURLOPT_RETURNTRANSFER => 1,
    CURLOPT_URL => $myUrl,
    CURLOPT_CUSTOMREQUEST => 'PATCH',
    CURLOPT_POSTFIELDS => $data
	));
	$resp = curl_exec($ch);
	curl_close($ch);
	//echo $resp;
	$data = json_decode($resp);
	$lev =  $data -> {'level'};
	if($lev == $val) $data = array('response' => '100');
	else $data = array('response' => '1');
    echo json_encode($data);

}

function updateLocation($binid , $lat , $lon)
{
	$myUrl = $GLOBALS['bin_url']."/{$binid}.json";
	$data = json_encode( array( $GLOBALS['S_LOC'] => array( $GLOBALS['S_LAT'] => $lat , $GLOBALS['S_LON'] => $lon) ) );
	$ch = curl_init();
	curl_setopt_array($ch, array(
    CURLOPT_RETURNTRANSFER => 1,
    CURLOPT_URL => $myUrl,
    CURLOPT_CUSTOMREQUEST => 'PATCH',
    CURLOPT_POSTFIELDS => $data
	));
	$resp = curl_exec($ch);
	curl_close($ch);
	//echo $resp;
	$data = json_decode($resp);
	$loc = $data -> {'location'};
	$lati = $loc -> {'lat'};
	$lng = $loc -> {'lng'};

	if($lati == $lat && $lng == $lon) $data = array('response' => '100');
	else $data = array('response' => '1');
	echo json_encode($data);

}


?>
