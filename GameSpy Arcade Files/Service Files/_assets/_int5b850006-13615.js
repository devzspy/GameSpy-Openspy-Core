<!DOCTYPE html>
<!-- Pi-hole: A black hole for Internet advertisements
*  (c) 2017 Pi-hole, LLC (https://pi-hole.net)
*  Network-wide ad blocking via your own hardware.
*
*  This file is copyright under the latest version of the EUPL. -->
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1"/>  <meta name="robots" content="noindex,nofollow"/>
  <meta http-equiv="x-dns-prefetch-control" content="off">
  <link rel="shortcut icon" href="http://pi.hole/admin/img/favicon.png" type="image/x-icon"/>
  <link rel="stylesheet" href="http://pi.hole/pihole/blockingpage.css" type="text/css"/>
  <title>● de.ign.com</title>
  <script src="http://pi.hole/admin/scripts/vendor/jquery.min.js"></script>
  <script>
    window.onload = function () {
      $("#bpBack").removeAttr("href");$("#bpWhitelist").prop("disabled", false);$("#bpWLPassword").attr("placeholder", "Password");$("#bpWLPassword").prop("disabled", false);    }
  </script>
</head>
<body id="blockpage"><div id="bpWrapper">
<header>
  <h1 id="bpTitle">
    <a class="title" href="/"></a>
  </h1>
  <div class="spc"></div>

  <input id="bpAboutToggle" type="checkbox"/>
  <div id="bpAbout">
    <div class="aboutPH">
      <div class="aboutImg"/></div>
      <p>Open Source Ad Blocker
        <small>Designed for Raspberry Pi</small>
      </p>
    </div>
    <div class="aboutLink">
      <a class="linkPH" href="https://github.com/pi-hole/pi-hole/wiki/What-is-Pi-hole%3F-A-simple-explanation"></a>
          </div>
  </div>

  <div id="bpAlt">
    <label class="altBtn" for="bpAboutToggle"></label>
  </div>
</header>

<main>
  <div id="bpOutput" class="hidden"></div>
  <div id="bpBlock">
    <p class="blockMsg">de.ign.com</p>
  </div>
    <div id="bpHelpTxt"><span/></div>
  <div id="bpButtons" class="buttons">
    <a id="bpBack" onclick="javascript:history.back()" href="about:home"></a>
    <label id="bpInfo" for="bpMoreToggle"></label>  </div>
  <input id="bpMoreToggle" type="checkbox">
  <div id="bpMoreInfo">
    <span id="bpFoundIn"><span>1</span>10</span>
    <pre id='bpQueryOutput'><span>[2]:</span>http://sysctl.org/cameleon/hosts
</pre>

    <form id="bpWLButtons" class="buttons">
      <input id="bpWLDomain" type="text" value="de.ign.com" disabled/>
      <input id="bpWLPassword" type="password" placeholder="Javascript disabled" disabled/><button id="bpWhitelist" type="button" disabled></button>
    </form>
  </div>
</main>

<footer><span>Tuesday 1:55 AM, August 28th.</span> Pi-hole v3.3.1-0-gfbee18e (raspberrypi/192.168.1.11)</footer>
</div>

<script>
  function add() {
    $("#bpOutput").removeClass("hidden error exception");
    $("#bpOutput").addClass("add");
    var domain = "de.ign.com";
    var pw = $("#bpWLPassword");
    if(domain.length === 0) {
      return;
    }
    $.ajax({
      url: "/admin/scripts/pi-hole/php/add.php",
      method: "post",
      data: {"domain":domain, "list":"white", "pw":pw.val()},
      success: function(response) {
        if(response.indexOf("Pi-hole blocking") !== -1) {
          setTimeout(function(){window.location.reload(1);}, 10000);
          $("#bpOutput").removeClass("add");
          $("#bpOutput").addClass("success");
        } else {
          $("#bpOutput").removeClass("add");
          $("#bpOutput").addClass("error");
          $("#bpOutput").html(""+response+"");
        }
      },
      error: function(jqXHR, exception) {
        $("#bpOutput").removeClass("add");
        $("#bpOutput").addClass("exception");
      }
    });
  }
      $(document).keypress(function(e) {
        if(e.which === 13 && $("#bpWLPassword").is(":focus")) {
            add();
        }
    });
    $("#bpWhitelist").on("click", function() {
        add();
    });
  </script>
</body></html>
