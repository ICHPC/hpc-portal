{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Welcome to the Imperial College Computing Portal</h1>


<p> Welcome to the Computing Portal run by the Imperial College High Performance Computing Service. You can use this web portal to run instances of packaged applications. 

<p>Create a new job by selecting <a href="?action=newjob">new job</a> from the menu to the left. Individual jobs may be associated with a particular <a href="?action=projects">project</a>. To see all jobs, past and present, select <a href="?action=joblist">job list</a>.

{if $admin_email}
<p>For information, contact <a href="mailto:{$admin_email}?subject={$admin_email_subject}">{$admin_name}</a>.
{/if}


<p> Current news for your pools:

<p>{$motd}


		</div>

{include file="footer.tpl"}
