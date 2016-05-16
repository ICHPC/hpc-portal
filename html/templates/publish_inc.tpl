{if $job_list[sec1].publish == "publish" }
    <td class="MYTABLE"><a href="{$url_base}?action=publish&amp;subaction=publish&amp;jid={$job_list[sec1].jid}">Publish</a></td>
{elseif $job_list[sec1].publish == "view" }
    <td class="MYTABLE">
    {if !empty($job_list[sec1].handle) }
            <a href="http://hdl.handle.net/{$job_list[sec1].handle}">Dspace</a></br>
    {/if}
    {if !empty($job_list[sec1].chempound) }
            <a href="{$job_list[sec1].chempound}">Chempound</a></br>
    {/if}
    {if !empty($job_list[sec1].figshare) }
            {if !empty($job_list[sec1].figshare_draft) && $job_list[sec1].figshare_draft=="1"}
                <a href="http://figshare.com/preview/_preview/{$job_list[sec1].figshare}">Figshare</a>&nbsp;<a href="{$url_base}?action=figsharepub&amp;jid={$job_list[sec1].jid}">(Publish)</a></br>
            {else}
                <a href="http://dx.doi.org/{$job_list[sec1].figshare}">Figshare</a></br>
            {/if}
    {/if}
    {if !empty($job_list[sec1].hpc_repo) }
                <a href="http://doi.org/{$job_list[sec1].hpc_repo}">HPC Repo</a></br>
    {/if}



    </td>
{elseif $job_list[sec1].publish == "na" }
    <td class="MYTABLE">---</td>
{else}
    <td class="MYTABLE"></td>
{/if}
