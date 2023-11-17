'use strict';
var express = require('express');
var router = express.Router();

router.get('/', function (req, res) {
    res.send("BOOMCARE SERVER.");
});

router.post('/light/ping', function (req, res) {
    console.log("== PING ==");
    console.log(req.body);
    res.send("END");
});

router.post('/light/temperature', function (req, res) {
    console.log("== TEMPERATURE ==");
    console.log(req.body);
    res.send("END");
});

module.exports = router;
