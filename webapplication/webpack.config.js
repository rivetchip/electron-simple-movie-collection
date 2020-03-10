const path = require('path');
const webpack = require('webpack')

const isDevelopment = (process.env.NODE_ENV === 'development');
const packageBuildPath = (isDevelopment ? 'devbuild' : 'releasebuild');

module.exports = {
    target: 'web',
    mode: (isDevelopment ? 'development' : 'production'),
    entry: {
        index: path.resolve('./index.html'),
        dashboard: path.resolve('./dashboard.js')
    },
    output: {
        publicPath: './',
        path: path.resolve(packageBuildPath),
        filename: '[name].bundle.js',
    },
    experiments: {
        asset: true
    },
    module: {
        rules: [
            {
                test: /\.(html)$/,
                // type: 'asset/source',
                // generator: {
                //     filename: '[name][ext]'
                // },
                use: [
                    {loader: 'file-loader', options: {
                        name: '[name].[ext]'
                    }},
                    {loader: 'extract-loader'},
                    {loader: 'html-loader', options: {
                        minimize: !isDevelopment,
                        attrs: [
                            'link:href', 'img:src'//, 'script:src'
                        ]
                    }}
                ],
            },
            {
                test: /\.(js|jsx)$/,
                exclude: /(node_modules)/,
                use: [
                    {'loader': 'babel-loader', options: {
                        // sourceType: 'module'
                        // cacheDirectory: '/tmp/babel-loader'
                    }}
                ]
            },
            {
                test: /\.(css)$/,
                use: [
                    {loader: 'file-loader', options: {
                        name: '[name].[ext]?[contenthash]'
                    }},
                    {loader: 'extract-loader'},
                    {loader: 'css-loader', options: {
                        sourceMap: true
                    }}
                ]
            },
            {
                // others assets
                test: /\.(svg|png|gif|jpg|woff|woff2|eot|ttf|otf)$/,
                use: [
                    {loader: 'file-loader', options: {
                        name: '[name].[ext]?[contenthash]'
                    }},
                ]
            },
            {
                test: /\.(txt|ndjson)$/,
                type: 'asset/resource',
                use: [
                    {loader: 'raw-loader'}
                ],
            },
        ]
    },
    plugins: [],
    devServer: {
        hot: true,
        overlay: true, // errors
        host: 'localhost', port: 8080,
        publicPath: '/',
        index: 'index.html',
        contentBase: path.resolve('.'),
        historyApiFallback: true,
        compress: true, // gzip
        // writeToDisk: true
    },
    devtool: 'source-map'
};
