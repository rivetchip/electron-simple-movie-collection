const path = require('path');
const webpack = require('webpack')


module.exports = {
    target: 'web',
    entry: {
        index: path.resolve('./index.html'),
        dashboard: path.resolve('./dashboard.js')
    },
    output: {
        path: path.resolve('./devbuild'),
        // filename: '[name].[hash].bundle.js',
        publicPath: './'
    },
    module: {
        rules: [
            {
                test: /\.(html)$/,
                use: [
                    {loader: 'file-loader', options: {
                        name: '[name].[ext]',
                    }},
                    {loader: 'extract-loader'},
                    {loader: 'html-loader', options: {
                        // minimize: true,
                        esModule: false,
                        attrs: ['link:href', 'img:src']
                    }}
                ],
            },
            {
                test: /\.(js|jsx)$/,
                exclude: /(node_modules)/,
                use: [
                    {'loader': 'babel-loader', options: {
                        // cacheDirectory: '/tmp/babel-loader'
                    }}
                ]
            },
            {
                test: /\.(css)$/,
                use: [
                    {loader: 'file-loader'},
                    {loader: 'extract-loader'},
                    {loader: 'css-loader', options: {
                        sourceMap: true
                    }}
                ]
            },
            {
                test: /\.(txt|ndjson)$/,
                use: [
                    {loader: 'raw-loader'}
                ],
            },
            {
                // others assets
                test: /\.(svg|png|gif|jpg|woff|woff2)$/,
                use: [
                    {loader: 'url-loader', options: {
                        limit: false, // base64
                        esModule: false
                    }}
                ]
            }
        ]
    },
    plugins: [
        // new webpack.HotModuleReplacementPlugin()
    ],
    devServer: {
        hot: true,
        overlay: true, // errors
        host: 'localhost', port: 8080,
        // publicPath: path.resolve('./'),
        contentBase: path.resolve('./devbuild'),
        compress: true,
        writeToDisk: true

    },
    devtool: 'source-map'
};
