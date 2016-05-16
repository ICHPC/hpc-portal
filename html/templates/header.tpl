<?xml version="1.0" encoding="iso-8859-1"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
<title>
Computing Portal
</title>

<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
<meta name="description" content="Portal Website" />
<meta name="keywords" content="" />
	
<link href="css/new_hm0.css" rel="stylesheet" type="text/css" />
        <link rel="stylesheet" type="text/css" href="css/small.css"  title="small" />
        <link href="css/core_css.css" rel="stylesheet" type="text/css" />
        <link href="css/local.css" rel="stylesheet" type="text/css" />
</head>


<body>




   <link rel="shortcut icon" href="images/portal-icon.ico" type="image/x-icon" />

	<div id="top">  
		<div id="topleft"> 
<!--			<a href="http://www.google.co.uk/" tabindex="1">Institution Home Page</a> -->
	  </div>
	  <div id="topright">
		{if !empty($gecos)}
		You are logged in as {$gecos}. <a href="?action=logout">Logout</a>
		{/if}
  		<!-- Text size selector -->
		</div>
	</div> <!--top-->


	<div class="spacer"></div>

<div class="contentwrap">

<div style="height: 64px;
width: 100%;
background-repeat: repeat;
margin: 0px;
padding: 0px;
clear: right;
float: left;
background-color: ;
background-image: url(images/bg.gif);">
<div style="background-image: url(images/tacky_compute_image.gif);
background-repeat: no-repeat;
background-position: right top;
margin: 0px;
padding: 0px;
z-index: 1;
width: 100%;
float: right;
height: 64px;
overflow: hidden;">
<div class="bottom_heading_notop">
<table align="left" ><tr>
<td class = "bottom_heading_large_notop" valign="bottom">
</td> 
<td class="bottom_heading_small_notop" valign="bottom">
<!-- for example
<a href="http://www.imperial.ac.uk/ict/services/hpc/highperformancecomputing"  style="color: #FFFFFF;text-decoration: none;">Imperial College Computing Portal</a>
-->
<a href=""  style="color: #FFFFFF;text-decoration: none;">HPC Service Computing Portal</a>
</td> 

</tr></table>

</div>
</div>
</div>


	<div id="leftmargin"> &nbsp; </div>  
	<div id="content">       
		<br />		  	  
		<!-- new hompage layout start -->
		<div id="hmLeftCol"><!-- Left Nav Start  -->
			<div id="navList">
				<div id="lselect">
					<ul>

{section name=sec1 loop=$menulinks}
   <li><a href="{$menulinks[sec1].url}">{$menulinks[sec1].name}</a></li>
{/section}

					</ul>
				</div> <!--lselect-->
			</div> <!--navlist-->
		</div> <!--hmleftcol-->

