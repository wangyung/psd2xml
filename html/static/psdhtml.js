if (window.$) $(function() {

var goToc = $('#go-toc'),
    layers = $('.is-a-layer'),
    previews = $('.background-overview');

function addButton(title, onClick) {
    return $('<div class="button">' + title + '</div>')
        .insertBefore(goToc)
        .click(onClick);
}

addButton('&nbsp;',
    function() {
        layers.toggleClass('background-grid');
    })
    .addClass('background-grid')
    .css({
        width: '16px',
        height: '16px'
    });

addButton('25%',
    function() {
        layers.removeClass('background-grid');
        previews.css('opacity', 0.25);
    }
);

addButton('75%',
    function() {
        layers.removeClass('background-grid');
        previews.css('opacity', 0.75);
    }
);

});

// vim: ts=4 sts=4 et
