<?php

function sendDiscordWebhook(string $webhookUrl, array $payload): bool
{
	if(empty($webhookUrl)) return false;

	$opt=['http'=>[
		'method'=>'POST',
		'header'=>'Content-Type: application/json',
		'content'=>json_encode($payload,JSON_UNESCAPED_SLASHES|JSON_UNESCAPED_UNICODE),
		'ignore_errors'=>true,
		'timeout'=>5,
	]];

	$response=@file_get_contents($webhookUrl,false,stream_context_create($opt));
	if($response===false)
	{
		error_log("Discord webhook failed: $webhookUrl");
		return false;
	}
	return true;
}

function discordWebhookForForum(string $where): string
{
	switch($where)
	{
		case 'show': return DISCORD_WEBHOOK_SHOW;
		case 'chat': return DISCORD_WEBHOOK_CHAT;
		case 'help': return DISCORD_WEBHOOK_HELP;
	}
	return '';
}
