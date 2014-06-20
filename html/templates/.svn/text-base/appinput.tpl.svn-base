{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Create new Job</h1>

			<h2>Upload Job Input</h2>


<form  enctype="multipart/form-data"  METHOD="POST">
<table class="MYTABLE">
<tr><th>Description</th><th>File</th></tr>

{section name=sec1 loop=$app_input}
<tr><td>{$app_input[sec1].description}
{if $app_input[sec1].required=='t'}
<font color=red>*</font>
{/if}
</td>
<td><input type="file" name="input-{$app_input[sec1].index}" size="50"></td>
</tr>
{/section}


<tr><td>Job description</td><td><input type="textfield" name="description" size="50"></td></tr></table>

</table>
<input type="hidden" name="action" value="newjob">
<input type="hidden" name="subaction" value="uploadinput">
<input type="hidden" name="project"  value = "{$project}" >
<input type="hidden" name="application"  value = "{$app}">
<input type="hidden" name="pool" value = "{$pool}" >
<input type="Submit" value="Submit" >

</form>

<p><font color=red>*</font> denotes required file



		</div>

{include file="footer.tpl"}
