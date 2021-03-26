<?php

/*

WARNING

You need to manually change the values of ?mode= parameter for production!
Make sure you know what you're doing. Make sure to check the bottom of this script!

*/

ini_set("display_errors", "off");
error_reporting(0);

$db = new mysqli("flagchecker_mysql", "stypr", "stypr", "stypr");
if($db->connect_errno) die("Crash");

function generateRandomString($length = 10) {
    $characters = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    $charactersLength = strlen($characters);
    $randomString = '';
    for ($i = 0; $i < $length; $i++) {
        $randomString .= $characters[rand(0, $charactersLength - 1)];
    }
    return $randomString;
}

/*

/?mode=generate&ip=[user_ip]&challenge_name=[challenge_name](&flag=[flag])

@return
    "Bingo{...}": on success
    "Crash": when the server crashes

@description
    This function records the flag.
    The flag is automatically generated when the flag value is not provided.
    This exists for special cases where you integrated the checker server without the kernel module.

*/
function generate($ip, $challenge_name, $flag=''){
    global $db;
    if($challenge_name){
        if(!$flag || strlen($flag) >= 73){
            $flag = "Bingo{" . hash("sha256", "1234bingo" . generateRandomString(32) . "styprzzz4321") . "}";
        }
        $ip = $db->real_escape_string($ip);
        $challenge_name = $db->real_escape_string($challenge_name);
        $date = $db->real_escape_string(date("Y-m-d H:i:s"));
        // Keep the flag name same if IP exists and matches.
        if($ip){
            $query = "SELECT * FROM flag_storage WHERE challenge_name='$challenge_name' AND ip='$ip';";
            $query = $db->query($query);
            if($res_log = $query->fetch_assoc()){
                return $res_log['flag'];
            }
        }
        $query = "INSERT INTO flag_storage (ip,challenge_name,flag,date) VALUES ('$ip', '$challenge_name', '$flag', '$date');";
        $query = $db->query($query);
        if($query){
            return $flag;
        }else{
            return "Crash";
        }
    }
}

/*

/?mode=verify&ip=[user_ip]&flag=[flag]&team_name=[team_name]&challenge_name=[challenge_name]

@return
    "challenge_name": when flag is correct on a given challenge
    "Crash": when the server crashes
    "": Internal Error

@description
    This function verifies the flag.
    There are some cheating detection available here.
*/
function verify($ip, $flag, $team_name, $challenge_name=''){
    global $db;
    if($flag){
        $ip = $db->real_escape_string($ip);
        $flag = $db->real_escape_string($flag);
        $team_name = $db->real_escape_string($team_name);
        $date = $db->real_escape_string(date("Y-m-d H:i:s"));

        // 1. Check Flag
        $flag_check = "SELECT * FROM flag_storage WHERE flag='$flag';";
        $flag_check = $db->query($flag_check);
        if($res = $flag_check->fetch_assoc()){
            // 2-1. Verify flag
            if(!($flag == $res['flag'])) return "";

            // 2-1-2. Verify challenge name. If the challenge_name is wrong, it means that the flag is invalid.
            if(!($challenge_name == $res['challenge_name'])) return "";

            // 2-2. Check if user already submitted a flag.
            $log_check = "SELECT * FROM flag_log WHERE flag='$flag' AND team_name='$team_name';";
            $log_check = $db->query($log_check);
            while($res_log = $log_check->fetch_array()){
                if($res_log['team_name'] == $team_name && $res_log['comment'] == "Success") return "";
            }

            // 2-3. Cheating: Violation if the team name does not match each other.
            $log_check = "SELECT * FROM flag_log WHERE flag='$flag';";
            $log_check = $db->query($log_check);
            if($res_log = $log_check->fetch_assoc()){
                // This should not return anything yet.
                if($res_log['team_name'] != $team_name){
                    $cheat_team_name = $db->real_escape_string($res_log['team_name']);
                    $cheat = "INSERT INTO flag_log (date,flag,team_name,ip,comment) VALUES ('$date', '$flag', '$team_name', '$ip', 'CHEAT: flag_trade: $team_name <-> $cheat_team_name');";
                    $cheat = $db->query($cheat);
                }
            }

            // 3. Insert success
            $query_succ = "INSERT INTO flag_log (date,flag,ip,team_name,comment) VALUES ('$date', '$flag', '$ip', '$team_name', 'Success');";
            $query_succ = $db->query($query_succ);

            // 3-1. Cheating: Violation if the same IP is used
            if($ip){
                $log_check = "SELECT COUNT(DISTINCT team_name) AS cheat_count, GROUP_CONCAT(DISTINCT team_name) AS cheat_team FROM flag_log WHERE ip='$ip';";
                $log_check = $db->query($log_check);
                if($res_log = $log_check->fetch_assoc()){
                    if((int)$res_log['cheat_count'] >= 2){
                        // This should not return anything yet.
                        $cheat_count = $db->real_escape_string($res_log['cheat_count']);
                        $cheat_team = $db->real_escape_string($res_log['cheat_team']);

                        // Stop spamming
            			$cheat_message = $db->real_escape_string("CHEAT: same_ip: [$ip] $cheat_count teams :: $cheat_team");
                        $spam_check = "SELECT COUNT(comment) AS check_count FROM flag_log WHERE comment='$cheat_message';";
                        $spam_check = $db->query($spam_check);
                        if($res_spam = $spam_check->fetch_assoc()){
                            if((int)$res_spam['check_count'] > 0){
                                // Skip it, or do something
                            }else{
                                $cheat = "INSERT INTO flag_log (date,flag,team_name,comment) VALUES ('$date', '$flag', '$team_name', 'CHEAT: same_ip: [$ip] $cheat_count teams :: $cheat_team');";
                                $cheat = $db->query($cheat);
                            }
                        }
                    }
                }
            }

            return $res['challenge_name'];
        }else{
            // 1-1. Logging invalid flags..
            $query = "INSERT INTO flag_log(date,flag,team_name,comment) VALUES ('$date', '$flag', '$team_name', 'Failed');";
            $query = $db->query($query);
            return "";
        }
    }
}

/*
Function made for debugging purposes.
Flushes the DB
*/
function flush_db(){
    global $db;
    $db->query("TRUNCATE TABLE flag_storage");
    $db->query("TRUNCATE TABLE flag_log");
}

/*
Function made for testing purposes.
Testing
*/
function test(){
    global $db;
    $db->query("TRUNCATE TABLE flag_storage");
    $db->query("TRUNCATE TABLE flag_log");
    $flag = generate("127.0.0.1", "simpleboard");
    $flag2 = generate("127.0.0.2", "simpleboard");
    $flag3 = generate("127.0.0.3", "simpleboard");
    echo "<pre>";
    var_dump("Flag1 for simpleboard@127.0.0.1: " . $flag);
    var_dump("Flag2 for simpleboard@127.0.0.2: " . $flag2);
    var_dump("Flag3 for simpleboard@127.0.0.4: " . $flag3);
    var_dump("Verify for Flag1@127.0.0.1:trader_team: " . verify("127.0.0.1", $flag, "trader_team", "simpleboard"));
    var_dump("Verify for Flag1@127.0.0.1:trader_team: " . verify("127.0.0.1", $flag, "trader_team", "simpleboard"));
    var_dump("Verify for Flag1@127.0.0.1:trader_team2: " . verify("127.0.0.1", $flag, "trader_team2", "simpleboard"));
    var_dump("Verify for Flag1@127.0.0.1:trader_team3: " . verify("127.0.0.1", $flag, "trader_team3", "simpleboard"));
    var_dump("Verify for Flag1@127.0.0.2:trader_team3: " . verify("127.0.0.2", $flag, "trader_team3", "simpleboard"));
    var_dump("Verify for Flag3@127.0.0.6:trader_team_first: " . verify("127.0.0.6", $flag2, "trader_team_first", "simpleboard"));
    var_dump("Verify for Flag3@127.0.0.6:trader_team_second: " . verify("127.0.0.6", $flag2, "trader_team_second", "simpleboard"));
    var_dump("Verify for Flag3@127.0.0.6:trader_team_third: " . verify("127.0.0.6", $flag, "trader_team_third", "simpleboard"));
    var_dump("Verify for Flag3@127.0.0.6:trader_team_third: " . verify("127.0.0.6", $flag3, "trader_team_third", "simpleboard"));
    var_dump("Verify for Flag3@127.0.0.6:trader_team: " . verify("127.0.0.1", $flag3, "trader_team", "asukaboard"));
    var_dump("Verify for Flag3@127.0.0.6:trader_team: " . verify("127.0.0.1", $flag3, "trader_team", "simpleboard"));

    echo "======================\n";
    echo "Did two teams cheat?\n";
    $cheating_check = $db->query("SELECT * FROM flag_log WHERE comment LIKE 'CHEAT:%'");
    while($row = $cheating_check->fetch_array()){
        var_dump($row['date'] . $row['comment']);
    }
    echo "</pre>";
}

function db_init(){
    global $db;
    $sql = <<< SQL
create table flag_storage(
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    ip VARCHAR(100) NOT NULL,
    challenge_name VARCHAR(255) NOT NULL,
    flag VARCHAR(255) NOT NULL,
    date DATETIME NOT NULL
);
SQL;
    $query = $db->query($sql);
    if(!$query) return;
    $sql = <<< SQL
create table flag_log(
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    date DATETIME NOT NULL,
    flag VARCHAR(255) NOT NULL,
    ip VARCHAR(255) NOT NULL,
    team_name TEXT NOT NULL,
    comment TEXT NOT NULL
);
SQL;
    $query = $db->query($sql);
}


db_init();
//test();
//flush_db();

// Template for admin
$template = "<!doctype html><html><head><title>cheating_check</title><meta name='robots' content='noindex,nofollow'><link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css'></head><body class='container-fluid'><table class='table table-bordered'>";

switch($_GET['mode']){
    // CHANGEME
    case "generate":
        die(generate($_GET['ip'], $_GET['challenge_name'], $_GET['flag']));

    // CHANGEME
    // This is for those challenges that integrated this server without the kernel module.
    // We seperated these from kernel modules to reduce the impact when the challenge is solved with an unexpected method.
    case "whalerice":
        if($_GET['challenge_name'] != "whalerice"){
            die("violation");
        }
        die(generate($_GET['ip'], $_GET['challenge_name'], $_GET['flag']));

    // CHANGEME
    case "verify":
        die(verify($_GET['ip'], $_GET['flag'], $_GET['team_name'], $_GET['challenge_name']));

    // CHANGEME
    case "admin_cheatcheck":
        global $db;
        $cheating_check = $db->query("SELECT * FROM flag_log WHERE comment LIKE 'CHEAT:%' ORDER BY date DESC");
        // Insert template
        $body = $template;
        $body .= "<thead><tr><th>date</th><th>ip</th><th>team_name</th><th>flag</th><th>log</th></tr></thead><tbody>";
        while($row = $cheating_check->fetch_array()){
            $body .= "<tr><td>" . $row['date'] . "</td><td>" . htmlspecialchars($row['comment']) . "</td></tr>";
        }
        $body .= "</tbody></table></body></html>";
        die($body);

    // CHANGEME
    case "admin_cheatcheck_all":
        // Checks for all records
        global $db;
        $flag_table = $db->query("SELECT * FROM flag_storage");
        $flag_dict = [];
        while($row = $flag_table->fetch_array()){
            $flag_dict[$row['flag']] = "<b>" . $row['challenge_name'] . "</b> from " . ($row['ip'] ? $row['ip'] : "(No IP)");
        }
        $cheating_check = $db->query("SELECT * FROM flag_log ORDER BY date DESC");
        // Insert template
        $body = $template;
        $body .= "<thead><tr><th>date</th><th>ip</th><th>team_name</th><th>flag</th><th>log</th></tr></thead><tbody>";
        while($row = $cheating_check->fetch_array()){
    	// echo $row['flag'];
	    $flag_info = @$flag_dict[$row['flag']];
            if(!$flag_info){
                 $flag_info = "<b>Invalid/Empty Flag</b>";
            }
            $body .= "<tr><td>" . $row['date'] . "</td><td>" . htmlspecialchars($row['ip']) . "</td><td>" . htmlspecialchars($row['team_name']) . "</td><td>" . $flag_info . "</td><td>" . htmlspecialchars($row['comment']) . "</td></tr>";
        }
        $body .= "</tbody></table></body></html>";
        die($body);

    default:
        die();
}

?>
