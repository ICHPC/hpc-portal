{if $job_list[sec1].publish == "publish" }
    <td class="MYTABLE"><a href="?action=publish&subaction=publish&jid={$job_list[sec1].jid}">Publish</a></td>
{elseif $job_list[sec1].publish == "view" }
    <td class="MYTABLE">
    {if !empty($job_list[sec1].handle) }
            <a href="http://hdl.handle.net/{$job_list[sec1].handle}">Dspace</a>
    {/if}
    {if !empty($job_list[sec1].chempound) }
            </br><a href="{$job_list[sec1].chempound}">Chempound</a>
    {/if}
    {if !empty($job_list[sec1].figshare) }
            {if !empty($job_list[sec1].figshare_draft) && $job_list[sec1].figshare_draft=="1"}
                </br><a href="http://figshare.com/preview/_preview/{$job_list[sec1].figshare}">Figshare</a>&nbsp;<a href="?action=figsharepub&jid={$job_list[sec1].jid}">(Publish)</a>
            {else}
                </br><a href="http://dx.doi.org/{$job_list[sec1].figshare}">Figshare</a>
            {/if}
    {/if}

    </td>
{elseif $job_list[sec1].publish == "na" }
    <td class="MYTABLE">---</td>
{else}
    <td class="MYTABLE"></td>
{/if}