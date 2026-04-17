

<?php
/**
 * discord.msg.send.php v0.8
 * Kirill Krasin &copy; 2025
 * https://github.com/Mo45
 *
 * For revisions and comments vist: https://gist.github.com/Mo45/cb0813cb8a6ebcd6524f6a36d4f8862c
 *
 * Sends a message to Discord via Webhook with file support and rate limit handling
 *
 * This function handles both simple messages and complex embeds with attachments,
 * automatically retrying on rate limits and validating file requirements.
 *
 * Create new webhook in your Discord channel settings and copy&paste URL into $webhookUrl.
 *
 * You can use Markdown in your message. More about formatting: https://discordapp.com/developers/docs/reference#message-formatting
 *
 * @param string $webhookUrl Full Discord webhook URL (from channel settings)
 * @param array $payload Message data array (content, embeds, username, etc.)
 * @param string|null $filePath Absolute path to file for attachment (optional)
 * @param int $maxRetries Maximum retry attempts on rate limits (default: 3)
 * @return bool True on success, false on failure (errors logged)
 */
function sendDiscordWebhook(string $webhookUrl, array $payload, ?string $filePath = null, int $maxRetries = 3): bool
{
    // =================================================================
    // FILE VALIDATION CHECKS
    // =================================================================
    // Verify file existence before processing
    if ($filePath && !file_exists($filePath)) {
        error_log("File not found: $filePath");
        return false;
    }

    // Enforce Discord's file size limit (8MB standard servers, 50MB boosted)
    if ($filePath && filesize($filePath) > 8 * 1024 * 1024) {
        error_log("File size exceeds Discord limit: $filePath");
        return false;
    }

    $retryCount = 0;

    // =================================================================
    // RATE LIMIT RETRY LOOP
    // =================================================================
    while ($retryCount <= $maxRetries) {
        $ch = curl_init($webhookUrl);

        // =============================================================
        // CURL BASE CONFIGURATION
        // =============================================================
        $options = [
            CURLOPT_FOLLOWLOCATION => true,   // Follow redirects (301/302)
            CURLOPT_HEADER => true,           // Include headers in response
            CURLOPT_RETURNTRANSFER => true,   // Return transfer instead of output
            CURLOPT_TIMEOUT => 120,           // Maximum execution time (seconds)
            CURLOPT_SSL_VERIFYPEER => true,   // Verify SSL certificate
            CURLOPT_POST => true,             // Always use POST method
        ];

        // =============================================================
        // PAYLOAD PREPARATION: FILE ATTACHMENT MODE
        // =============================================================
        if ($filePath) {
            // Generate unique boundary for multipart separation
            $boundary = '----DiscordWebhookBoundary' . uniqid();

            // Encode metadata payload to JSON
            $payloadJson = json_encode($payload, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE);

            // =========================================================
            // MULTIPART REQUEST BODY CONSTRUCTION
            // =========================================================
            // Part 1: JSON metadata payload
            $body = "--$boundary\r\n";
            $body .= "Content-Disposition: form-data; name=\"payload_json\"\r\n";
            $body .= "Content-Type: application/json\r\n\r\n";
            $body .= $payloadJson . "\r\n";

            // Part 2: File attachment
            $body .= "--$boundary\r\n";
            $body .= "Content-Disposition: form-data; name=\"file\"; filename=\"" . basename($filePath) . "\"\r\n";
            $body .= "Content-Type: " . mime_content_type($filePath) . "\r\n\r\n";
            $body .= file_get_contents($filePath) . "\r\n";

            // End boundary marker
            $body .= "--$boundary--\r\n";

            // =========================================================
            // CURL CONFIG FOR FILE UPLOAD
            // =========================================================
            $options[CURLOPT_HTTPHEADER] = [
                "Content-Type: multipart/form-data; boundary=$boundary",
                "Content-Length: " . strlen($body)
            ];
            $options[CURLOPT_POSTFIELDS] = $body;
        }
        // =============================================================
        // PAYLOAD PREPARATION: JSON-ONLY MODE
        // =============================================================
        else {
            // Standard JSON request configuration
            $options[CURLOPT_HTTPHEADER] = ['Content-Type: application/json'];
            $options[CURLOPT_POSTFIELDS] = json_encode($payload, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE);
        }

        // Apply all configured options
        curl_setopt_array($ch, $options);

        // =============================================================
        // EXECUTE CURL REQUEST
        // =============================================================
        $response = curl_exec($ch);

        // =============================================================
        // CURL ERROR HANDLING
        // =============================================================
        if ($response === false) {
            $error = curl_error($ch);
            $errno = curl_errno($ch);
            curl_close($ch);
            error_log("cURL error ($errno): $error");
            return false;
        }

        // =============================================================
        // RESPONSE PROCESSING
        // =============================================================
        $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        $headerSize = curl_getinfo($ch, CURLINFO_HEADER_SIZE);
        $responseHeaders = substr($response, 0, $headerSize);
        $responseBody = substr($response, $headerSize);

        curl_close($ch);

        // =============================================================
        // SUCCESSFUL DELIVERY (2xx STATUS)
        // =============================================================
        if ($httpCode >= 200 && $httpCode < 300) {
            return true;
        }

        // =============================================================
        // RATE LIMIT HANDLING (429 TOO MANY REQUESTS)
        // =============================================================
        if ($httpCode === 429) {
            $retryAfter = 1;  // Default cooldown (in seconds)

            // Parse Retry-After header (seconds to wait)
            if (preg_match('/retry-after: (\d+)/i', $responseHeaders, $matches)) {
                $retryAfter = (int)$matches[1];
            }
            // Fallback to Discord's rate-limit header
            elseif (preg_match('/x-ratelimit-reset-after: (\d+\.?\d*)/i', $responseHeaders, $matches)) {
                $retryAfter = (float)$matches[1];
            }

            error_log("Rate limit hit. Retrying after {$retryAfter} seconds");
            sleep(ceil($retryAfter) + 1);  // Add safety buffer
            $retryCount++;
            continue;
        }

        // =============================================================
        // ERROR LOGGING AND ANALYSIS
        // =============================================================
        $errorInfo = [
            'http_code' => $httpCode,
            'response' => $responseBody,
            'headers' => $responseHeaders,
            'file' => $filePath ? basename($filePath) : 'none'
        ];

        error_log("Discord API error: " . print_r($errorInfo, true));

        // =============================================================
        // FATAL ERROR HANDLING (4xx CLIENT ERRORS)
        // =============================================================
        // Don't retry on permanent client errors (except 429)
        if ($httpCode >= 400 && $httpCode < 500 && $httpCode !== 429) {
            return false;
        }

        $retryCount++;
    }

    // =================================================================
    // FINAL FAILURE AFTER RETRIES
    // =================================================================
    error_log("Failed after $maxRetries attempts");
    return false;
}

// // =====================================================================
// // PASTE YOUR WEBHOOK URL HERE
// // =====================================================================
// $webhookUrl = "https://discordapp.com/api/webhooks/1494725746395185323/mtUNd7-MuFAbJNv2IMt4B6id5SWMx_ZthAHrBRhFb9RTh8uPXNxltG8csCWVLiwDr6Qx";

// // =====================================================================
// // MESSAGE CONFIGURATION
// // =====================================================================
// // Base message parameters
// $message = [
// 	// Text content
//     'content' => "This is message with mention, use userID <@12341234123412341>",
//     // Custom username
//     // 'username' => "php-msg-bot",
//     // Text-to-speech
//     'tts' => false,
//     // // Mention permissions
//     // 'allowed_mentions' => [
//     //     'parse' => ['users'] // Allow @user mentions
//     // ]
// ];

// // =====================================================================
// // EMBED CONSTRUCTION
// // =====================================================================
// $embed = [
//     'title' => "Annotated Discord Webhook",               // Embed title
//     'type' => "rich",                                     // Embed type
//     'description' => "Embed with detailed documentation", // Main text
//     'url' => "https://gist.github.com/Mo45/cb0813cb8a6ebcd6524f6a36d4f8862c", //URL of title link
//     'timestamp' => date(DATE_ATOM),                       // ISO 8601 timestamp
//     'color' => hexdec("3366ff"),                          // Left border color
//     'footer' => [                                         // Footer section
//         'text' => "GitHub.com/Mo45",
//         'icon_url' => "https://avatars.githubusercontent.com/u/4648775"
//     ],
//     'author' => [                                         // Author section
//         'name' => "Mo45",
//         'url' => "https://github.com/Mo45"
//     ],
//     'fields' => [                                         // Data fields
//         [
//             'name' => "New Feature",
//             'value' => "File upload",
//             'inline' => true
//         ],
//         [
//             'name' => "Status",
//             'value' => "Completed",
//             'inline' => true
//         ]
//     ]
// ];

// // Attach embed to message
// $message['embeds'] = [$embed];

// // =====================================================================
// // FILE ATTACHMENT (OPTIONAL)
// // =====================================================================
// // Uncomment to enable file upload:
// // $filePath = __DIR__ . '/test.jpg';

// // =====================================================================
// // EXECUTION
// // =====================================================================
// $result = sendDiscordWebhook(
//     webhookUrl: $webhookUrl,
//     payload: $message,
//     filePath: $filePath ?? null,  // Pass null if no file
//     maxRetries: 5
// );

// if ($result) {
//     echo "Message delivered successfully!";
// } else {
//     echo "Delivery failed. Check error logs for details.";
// }
