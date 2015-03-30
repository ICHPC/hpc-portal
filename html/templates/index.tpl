{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Welcome to the Computing Portal</h1>


<p> Welcome to the Computing Portal. You can use this web portal to run instances of packaged applications.

<p>Create a new job by selecting <a href="?action=newjob">new job</a> from the menu to the left. Individual jobs may be associated with a particular <a href="?action=projects">project</a>. To see all jobs, past and present, select <a href="?action=joblist">job list</a>.

{if $admin_email}
<p>For information, contact <a href="mailto:{$admin_email}?subject={$admin_email_subject}">{$admin_name}</a>.
{/if}


<p> Current news for your pools:

<p>{$motd}

{if $job_list}
<p>The following jobs have passed their embargoes and need
<a href="?action=joblist&embargoed=3&orderby=6&orderdir=0&status=3">
attention</a>.</p>
<table class="MYTABLE">
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
        <td>{$job_list[sec1].jid}</td>
        <td>{$job_list[sec1].app_name}</td>
        <td>{$job_list[sec1].description}</td>
        <td>{$job_list[sec1].submit_time}</td>
        <td><a href="?action=delete&jid={$job_list[sec1].jid}">Delete</a></td>

        {include file="publish_inc.tpl"}
        <td><a href="?action=cancelembargojob&jid={$job_list[sec1].jid}">
            Cancel</a></td>
        <td>{$job_list[sec1].embargo_date}</td>
    </tr>
{/section}
</table>
{/if}

</div>

{include file="footer.tpl"}
