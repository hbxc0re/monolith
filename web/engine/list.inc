<?

 /*
  * $id$
  */

function list_quads($username) {

    // Check user priv.
    //
    switch(check_priv($username,0)) {

        case -1:
            PRINT "<h3 align=\"center\">You are not currently logged in. Log in first.</h3>\n";
            return;
        default:
            break;
    }

    $usernum = get_usernum($username);

    connect_db();

    $result = MYSQL_QUERY("SELECT ut.forum_id AS id,f.name AS name,COUNT(m.message_id) AS total,(MAX(m.message_id)-ut.lastread) AS unread,uf.status AS status FROM usertopic as ut LEFT JOIN message as m ON (m.forum_id=ut.forum_id) LEFT JOIN forum as f ON ut.forum_id=f.id LEFT JOIN userforum AS uf ON (uf.forum_id=ut.forum_id AND uf.user_id=ut.user_id) WHERE ut.user_id=$usernum AND status='invited' GROUP BY ut.forum_id;");
    $number = MYSQL_NUMROWS($result);
    if($number) {

        PRINT "<h3 align=\"center\">Listing $number unzapped Quadrants.</h3>\n";
        PRINT "<table border=\"0\" cellpadding=\"2\" cellspacing=\"2\" align=\"center\">\n";
        $i = 0;

        while ($i < $number) {
            $id = mysql_result($result,$i,"id");
            $name = mysql_result($result,$i,"name");
            $total = mysql_result($result, $i,"total");
            $unread = mysql_result($result, $i,"unread");
            $status = mysql_result($result,$i,"status");
            $name = htmlspecialchars($name);
           
            // Show quad with message info.
            //
            PRINT "<tr>\n";
            PRINT "  <td align=\"left\">\n";
            PRINT "    <p align=\"left\">$id.\n";
            PRINT "  </td>\n";
            PRINT "  <td align=\"left\">\n";
            PRINT "    <p align=\"left\"><a href=\"/bbs/index.phtml/l/$id\"><font color=\"#ffff00\">$name</font></a>\n";
            PRINT "  </td>\n";
            PRINT "  <td align=\"left\">\n";
            if ($unread == NULL || $unread  <= 0)
                PRINT "    <p align=\"left\">(<font color=\"#00cc00\">$total messages, 0 unread</font>)<br>\n";
            else {
                if($unread > $total)
                    $unread = $total;
                PRINT "    <p align=\"left\">(<font color=\"#00cc00\">$total messages,</font> <font color=\"#ffff00\">$unread unread</font>)<br>\n";
            }
            PRINT "  </td>\n";
            PRINT "</tr>\n";
            $i++;
        }

        PRINT "</table>\n";
    } else {
        PRINT "<h3 align=\"center\">No quadrants found.</h3>\n";
    }
}

?>