{
	"geode": "4.6.0",
	"gd": {
		"win": "2.2074",
		"android": "2.2074",
		"mac": "2.2074",
		"ios": "2.2074"
	},
	"id": "user.autobot",
	"name": "AutoBot",
	"version": "v1.0.0",
	"developer": "you",
	"description": "Macro recorder / auto-play bot with an optional anti-death safety net. Never submits progress while active.",
	"settings": {
		"safety-net": {
			"type": "bool",
			"default": true,
			"name": "Anti-death safety net (Replay only)",
			"description": "While replaying, ignore deaths so a tiny desync does not kill the run. Progress is never saved while the bot is active."
		}
	}
}