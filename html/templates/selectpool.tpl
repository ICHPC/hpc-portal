{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Create new Job</h1>

{if $pools}

			<h2>Select Pool</h2>


<form>
<select name="pool">
{html_options values=$pool_idx  output=$pools selected=$default_pool_idx}
</select>

<input type="hidden" name="action" value="newjob">
<input type="hidden" name="subaction" value="selectapp">

<p>
<input type="Submit" value="Continue" >

</form>
{else}
<P>No compute pools available.
{/if}




		</div>

{include file="footer.tpl"}
