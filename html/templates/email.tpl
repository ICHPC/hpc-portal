<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
<title>
Embargoed Jobs
</title>

<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
</head>

<body>
<p>The following jobs have passed their embargoes and need
<a href="{$url_base}?action=joblist&amp;embargoed=3&amp;orderby=6&amp;orderdir=0&amp;status=3">
attention</a>.</p>
<table border="1">
<thead>
<tr>	
    <th> Job ID</th>
    <th> Application</th>
    <th> Description</th>
    <th> Submission Time</th>
    <th> Delete</th>
    <th> Repository</th>
    <th> Cancel Embargo</th>
    <th> Embargo Date</th>
</tr>
</thead>

{section name=sec1 loop=$job_list}
    <tr>
        <td><a href="{$url_base}?action=editjob&amp;jid={$job_list[sec1].jid}">{$job_list[sec1].jid}</a></td>
        <td>{$job_list[sec1].app_name}</td>
        <td>{$job_list[sec1].description}</td>
        <td>{$job_list[sec1].submit_time}</td>
        <td><a href="{$url_base}?action=delete&amp;jid={$job_list[sec1].jid}">Delete</a></td>

        {include file="publish_inc.tpl"}
        <td><a href="{$url_base}?action=cancelembargojob&amp;jid={$job_list[sec1].jid}">
            Cancel</a></td>
        <td>{$job_list[sec1].embargo_date}</td>
    </tr>
{/section}
</table>
</body>
</html>
