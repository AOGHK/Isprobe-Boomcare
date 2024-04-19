'use strict';
var express = require('express');
var router = express.Router();
const axios = require('axios');
const https = require('https');

router.get('/', function (req, res) {
    res.send("BOOMCARE SERVER.");
});

router.post('/pingAPI', async (req, res) => {
    // At request level
    const agent = new https.Agent({
        rejectUnauthorized: false
    });

    axios({
        httpsAgent: agent,
        method: "post", // ��û ���
        url: "https://asia-northeast2-dadadak-f5f84.cloudfunctions.net/temperatureAPI", // ��û �ּ�
        data: {
            data: [{
                mac: "EC:DA:3B:D5:35:8A",
                temp: "36.10",
                time: "2024-04-19 18:10:10"
            }]            
        } // ���� ������(body)
    }).then(function (response) {
        console.log(response.data);
    }).catch(function (err) {
        console.log(err); // ���� ó�� ����
    });
    res.send("END");
});

router.post('/temperatureAPI', function (req, res) {
    console.log("== TEMPERATURE ==");
    console.log(req.body);
    res.send("END");
});


module.exports = router;
